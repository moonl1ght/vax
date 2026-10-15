#include "debugView.h"
#include "viewManager.h"

using namespace vax::ui;

void DebugView::_onClose() { _viewManager->popView(); }
