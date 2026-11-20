#include "debugView.h"
#include "viewManager.h"

using namespace vax::app;

void DebugView::_onClose() { _viewManager->popView(); }
