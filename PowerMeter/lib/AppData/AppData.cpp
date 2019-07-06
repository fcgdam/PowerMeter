#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <AppData.hpp>

AppData appData;

void AppData::setFWVersion( String fwversion ) {
    m_FWVersion = fwversion;
}

void AppData::setLogServerIPInfo( String lsIP ) {
    m_logServerIP = lsIP;
}

void AppData::setPZEMState( uint8_t state ) {
    m_PZEMState = state;
}

String AppData::getPZEMState() {
    switch ( m_PZEMState) {
        case PZEM_DISCONNECTED: return String("Not Connected");
        case PZEM_CONNECTING:   return String("Connecting");
        case PZEM_CONNECTED:    return String("Connected");
        case PZEM_CONNECTFAIL:  return String("Connect Failed");
        default: return ("State Error");
    }
}

void AppData::setVoltage(float Vin) {
    m_V = Vin;
}

String AppData::getVoltage() {
    return String(m_V);
}

void AppData::setCurrent(float Iin) {
    m_I = Iin;
}

String AppData::getCurrent() {
    return String(m_I);
}

void AppData::setPower(float Pin) {
    m_P = Pin;
}

String AppData::getPower() {
    return String(m_P);
}

void AppData::setEnergy(float Ein) {
    m_E = Ein;
}

String AppData::getEnergy() {
    return String(m_E);
}

void AppData::setSamplesOK() {
    m_samplesOK++;
}

void AppData::setSamplesNOK() {
    m_samplesNOK++;
}

String AppData::getSamplesOK() {
    return String(m_samplesOK);
}

String AppData::getSamplesNOK() {
    return String(m_samplesNOK);
}

String AppData::getFWVersion() {
    return m_FWVersion;
}

String AppData::getSSID() {
    return WiFi.SSID();
}

String AppData::getRSSI() {
    return String(WiFi.RSSI());
}

String AppData::getDevIP() {
    IPAddress ipdev = WiFi.localIP();

    return ipdev.toString();
}

String AppData::getGWIP() {
    IPAddress ipgw = WiFi.gatewayIP();

    return ipgw.toString();
}

String AppData::getLogServerIPInfo() {
    return m_logServerIP;
}

String AppData::getHeap() {
    return String(ESP.getFreeHeap());
}