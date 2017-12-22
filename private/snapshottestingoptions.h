#ifndef SNAPSHOTTESTINGOPTIONS_H
#define SNAPSHOTTESTINGOPTIONS_H

namespace SnapshotTesting {


    /// Options for capture
    class Options {
    public:

        inline Options() {
            captureVisibleItemOnly = true;
            expandAll = false;
            hideId = false;
            indentSize = 4;
            captureOnReady = true;
        }

        bool captureVisibleItemOnly;
        bool expandAll;
        bool hideId;
        int indentSize;

        /// Capture only if the component is ready (Loader and Image are loaded completely)
        bool captureOnReady;
    };


}

#endif // SNAPSHOTTESTINGOPTIONS_H
