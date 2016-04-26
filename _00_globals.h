struct CloudConfig {
 
  unsigned int publishRateInSeconds = 60; // defaults to once a minute
  // WARNING EXPIRY SET TO 10 YEARS FROM NOW.  
  // Epoch Timestamp Conversion Tool http://www.epochconverter.com/
  // Expires Wed, 22 Jan 2025 00:00:00 GMT.  Todo: add expiry window - eg now plus 2 days...
  // IOT HUB Devices can be excluded by device id/key - expiry window not so relevant
  // EVENT Hubs Devices can only be excluded by policy so a more sensible expiry should be tried and you'd need to device a moving expiry window
  unsigned int sasExpiryDate = 1737504000;  // Expires Wed, 22 Jan 2025 00:00:00 GMT
  const char *host;
  char *key;
  const char *id;
  const char *geo;
  unsigned long lastPublishTime = 0;
  String fullSas;
  String endPoint;
};

