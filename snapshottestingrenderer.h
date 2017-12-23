#ifndef SNAPSHOTTESTINGRENDERER_H
#define SNAPSHOTTESTINGRENDERER_H

#include <QQmlEngine>
#include <QImage>
#include <QPointer>
#include <QQuickWindow>
#include <QOffscreenSurface>
#include <QQuickRenderControl>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>

namespace SnapshotTesting {

    class Renderer {

    public:
        Renderer(QQmlEngine* engine);
        ~Renderer();

        QPointer<QQmlEngine> engine() const;

        bool load(const QString& source);

        QString snapshot() const;

        QImage screenshot() const;

    private:
        QPointer<QQmlEngine> m_engine;

        QString m_snapshot;

        QImage m_screenshot;

        /* Internal variables */
        QWindow *owner;
        QQuickWindow* window;
        QQuickRenderControl* renderControl;
        QOffscreenSurface* surface;
        QOpenGLContext *context;
        QOpenGLFramebufferObject *fbo;
    };

}

#endif // SNAPSHOTTESTINGRENDERER_H
