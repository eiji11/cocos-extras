# UILockManager
cocos2d-x's touch detection mechanism doesn't consider modal nodes covering a node with touch event.
Imagine a button covered by a dialog. Touching the location of the supposed to be button will still dispatch its touch event.

UILockManager takes care of that problem by setting up lock level for a node with touch event.