#include <QtQuickTest/quicktest.h>
#include <snapshot_init.h>

int main(int argc, char **argv)
{
    QApplication a{argc,argv};
    SnapshotTesting::init();
    Q_INIT_RESOURCE(snapshottesting);
    return quick_test_main(argc, argv, "Example1", QUICK_TEST_SOURCE_DIR);
}
