#include "debugView.h"
#include "viewManager.h"

using namespace vax::editor;

void DebugView::_onClose() { _viewManager->popView(); }
