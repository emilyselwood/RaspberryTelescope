#ifndef __TELESCOPE_H
#define __TELESCOPE_H

void signalTelescope(const char *address, const char *port, const int axis);

void serialTelescope(const char *port, const int axis);

#endif
