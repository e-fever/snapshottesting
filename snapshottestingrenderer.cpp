#include <QOpenGLFunctions>
#include <QQuickItem>
#include <asyncfuture.h>
#include <aconcurrent.h>
#include "snapshottestingrenderer.h"
#include "snapshottesting.h"

SnapshotTesting::Renderer::Renderer(QQmlEngine* engine) : m_engine(engine)
{

    class RenderControl : public QQuickRenderControl
    {
    public:
        RenderControl(QWindow *w) : m_window(w) { }
        QWindow *renderWindow(QPoint *offset) Q_DECL_OVERRIDE {
            if (offset)
                *offset = QPoint(0, 0);
            return m_window;
        }

    private:
        QWindow *m_window;
    };

    owner = new QWindow();
    renderControl = new RenderControl(owner);
    window = new QQuickWindow(renderControl);

    surface = new QOffscreenSurface();
    context = new QOpenGLContext();

    QSurfaceFormat format;
    format.setDepthBufferSize(16);
    format.setStencilBufferSize(8);
    context->setFormat(format);
    context->create();

    surface->setFormat(format);
    surface->create();

    fbo = 0;
}

SnapshotTesting::Renderer::~Renderer()
{
    context->makeCurrent(surface);

    delete owner;
    delete window;
    delete renderControl;

    if (fbo) {
        delete fbo;
    }

    context->doneCurrent();

    delete surface;
    delete context;
}

bool SnapshotTesting::Renderer::load(const QString &source)
{
    // Create the object
    auto _create = [=](const QString &source) -> QObject* {
        QUrl url = QUrl::fromLocalFile(source);
        QQmlComponent component(m_engine.data(), url);

        if (component.isError()) {
            const QList<QQmlError> errorList = component.errors();

            for (const QQmlError &error : errorList) {
                qWarning() << error.url() << error.line() << error;
            }
            return 0;
        }

        return component.create();
    };

    auto defer = AsyncFuture::deferred<void>();

    auto _render = [=]() {
        auto d = defer;

        if (!context->makeCurrent(surface)) {
            qDebug() << "Failed to render";
            d.cancel();
            return;
        }

        renderControl->polishItems();
        renderControl->sync();
        renderControl->render();

        window->resetOpenGLState();

        QOpenGLFramebufferObject::bindDefault();

        context->functions()->glFlush();

        m_screenshot = fbo->toImage();

        d.complete();
    };

    auto _init = [=](QObject* rootObject) {
        auto d = defer;
        QQuickItem* rootItem = qobject_cast<QQuickItem *>(rootObject);

        if (!rootItem) {
            d.cancel();
            return;
        }

        QObject::connect(window, &QQuickWindow::sceneGraphInitialized, [=]() mutable {

            fbo = new QOpenGLFramebufferObject(window->size(), QOpenGLFramebufferObject::CombinedDepthStencil);
            window->setRenderTarget(fbo);
            _render();
        });

        qreal width = rootItem->width();
        qreal height = rootItem->height();

        if (width == 0 || height == 0) {
            qWarning() << "render: Item's width or height is zero";
            d.cancel();
            return;
        }

        rootItem->setParentItem(window->contentItem());
        window->setGeometry(0,0,width, height);

        context->makeCurrent(surface);
        renderControl->initialize(context);
    };

    QObject* rootObject = _create(source);
    rootObject->setParent(owner);

    if (!rootObject) {
        return false;
    }

    //@TODO - Options
    m_snapshot = SnapshotTesting::capture(rootObject);

    QQuickItem* rootItem = qobject_cast<QQuickItem *>(rootObject);
    if (!rootItem) {
        return true;
    }

    _init(rootItem);

    AConcurrent::await(defer.future());

    return true;
}

QImage SnapshotTesting::Renderer::screenshot() const
{
    return m_screenshot;
}

QString SnapshotTesting::Renderer::snapshot() const
{
    return m_snapshot;
}

QPointer<QQmlEngine> SnapshotTesting::Renderer::engine() const
{
    return m_engine;
}
