#define HTTP 1

#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <libusb-1.0/libusb.h>
#include <labnation.h>
#include <labnation/smartscopeusb.h>
#ifdef HTTP
#include <labnation/httpserver.h>
#else
#include <labnation/interfaceserver.h>
#endif

using namespace labnation;

libusb_context* usb_ctx;
libusb_device** devices;
libusb_device_descriptor desc;
SmartScopeUsb* scope;
#ifdef HTTP
HttpServer* server;
#else
InterfaceServer* server;
#endif

int main(int argc, char *argv[])
{
  libusb_init(NULL);
  //libusb_set_debug(usb_ctx, LIBUSB_LOG_LEVEL_DEBUG);
#if HTTP
  server = new HttpServer();
  server->Start();
  delete(server);
#else
  while(true)
  {
    int n = libusb_get_device_list(NULL, &devices);
    for(int i = 0; i < n; i++) {
      libusb_get_device_descriptor(devices[i], &desc);
      if(desc.idVendor == SmartScopeUsb::VID) {
        for(std::vector<int>::const_iterator j = SmartScopeUsb::PIDs.begin();
            j != SmartScopeUsb::PIDs.end(); ++j) {
          if(desc.idProduct == *j) {

            scope = new SmartScopeUsb(devices[i]);
            server = new InterfaceServer(scope);
            server->Start();
            while(server->GetState() != InterfaceServer::State::Destroyed) {
              std::this_thread::sleep_for(std::chrono::milliseconds(100));
              if(server->GetState() == InterfaceServer::State::Stopped)
                server->Start();
            }
            debug("Server destroyed - quitting");
            delete(server);
            delete(scope);
          }
        }
      }
    }
    libusb_free_device_list(devices, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }
#endif
  libusb_exit(NULL);
}
