#include "menuView.h"
#include "viewManager.h"

using namespace vax::editor;

void MenuView::_onClose() { _viewManager->popView(); }
