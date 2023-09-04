#ifndef WebPortal_h
#define WebPortal_h

#include <functional>
#include <Arduino.h>
#include "Web/VictorWeb.h"

namespace Victor::Components {
  class WebPortal : public VictorWeb {
   public:
    WebPortal(int port = 80);
    ~WebPortal();

   private:
    void _registerHandlers() override;
    void _solvePageTokens(String& html) override;
    void _handleSampleGet();
    void _handleSamplePost();
  };
} // namespace Victor::Components

#endif // WebPortal_h
