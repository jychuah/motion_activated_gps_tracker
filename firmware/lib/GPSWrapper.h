#include "Adafruit_GPS.h"
#include "ParticleFtpClient.h"


#define STATE_SPINNING_UP -1
#define STATE_GET_EPO_STATUS 0
#define STATE_WAIT_FOR_EPO_STATUS 1
#define STATE_STREAM_EPO 2
#define STATE_SEND_TIME_HINT 3
#define STATE_WAIT_FOR_TIME_HINT_SUCCESS 4
#define STATE_SEND_LOCATION_HINT 5
#define STATE_WAIT_FOR_LOCATION_HINT_SUCCESS 6
#define STATE_BOOTED 7
#define NMEA_UPDATES_BEFORE_RETRY 5

using namespace particleftpclient;


class GPSWrapper {
public:
  Adafruit_GPS gps = Adafruit_GPS();
  void begin();
  void run();
  int getState();

private:
  ParticleFtpClient ftp = ParticleFtpClient();

  bool streamEpo();
  void sendTimeHint();
  void sendGpsHint();
  void sendEpoRequest();
  void runStateMachine();

  String hostname = "ftp.gtop-tech.com";
  String username = "gtopagpsenduser01";
  String password = "enduser080807";
  String MTKfilename = "MTK7d.EPO";
  int timeout = 8000;
  long lastTimeout = 0;
  int state = STATE_SPINNING_UP;
  int counter = 0;
};
