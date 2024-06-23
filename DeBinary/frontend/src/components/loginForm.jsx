import React, { useState, useEffect } from "react";

const AddDeviceForm = () => {
  const [ssid, setSSID] = useState("");
  const [password, setPassword] = useState("");
  const [device, setDevice] = useState(null);
  const [server, setServer] = useState(null);
  const [characteristic, setCharacteristic] = useState(null);
  const [scanResults, setScanResults] = useState([]);

  // UUIDs
  const deviceName = "ESP32";
  const serviceUUID = "19b10000-e8f2-537e-4f6c-d104768a1214";
  const characteristicUUID = {
    control: "19b10002-e8f2-537e-4f6c-d104768a1214",
    wifiScan: "19b10003-e8f2-537e-4f6c-d104768a1214",
  };

  // Function to check if Web Bluetooth API is supported
  const isWebBluetoothEnabled = () => {
    if (!navigator.bluetooth) {
      console.log("Web Bluetooth API is not available in this browser!");
      return false;
    }
    console.log("Web Bluetooth API supported in this browser.");
    return true;
  };

  // Function to connect to the ESP32 device
  const connectToDevice = async () => {
    if (!isWebBluetoothEnabled()) return;

    try {
      console.log("Initializing Bluetooth...");
      const device = await navigator.bluetooth.requestDevice({
        filters: [{ name: deviceName }],
        optionalServices: [serviceUUID],
      });

      const server = await device.gatt.connect();
      const service = await server.getPrimaryService(serviceUUID);
      const characteristicControl = await service.getCharacteristic(characteristicUUID.control);
      const characteristicWifiScan = await service.getCharacteristic(characteristicUUID.wifiScan);

      // Set state for device, server, and characteristic
      setDevice(device);
      setServer(server);
      setCharacteristic({
        control: characteristicControl,
        wifiScan: characteristicWifiScan,
      });

      console.log("Device connected:", device);
    } catch (error) {
      console.log("Error:", error);
    }
  };

  // Function to start WiFi scan
  const startWifiScan = async () => {
    if (!characteristic || !characteristic.wifiScan) {
      console.error("No WiFi scan characteristic available. Connect to the device first.");
      return;
    }

    try {
      // Start notifications for characteristicvaluechanged event
      await characteristic.wifiScan.startNotifications();

      // Handle characteristicvaluechanged event
      characteristic.wifiScan.addEventListener("characteristicvaluechanged", handleCharacteristicValueChanged);

      console.log("WiFi scan started.");
    } catch (error) {
      console.error("Error starting WiFi scan:", error);
    }
  };

  // Function to handle characteristicvaluechanged event
  const handleCharacteristicValueChanged = (event) => {
    const value = event.target.value;
    const decoder = new TextDecoder("utf-8");
    const jsonString = decoder.decode(value);

    try {
      const scanResults = JSON.parse(jsonString);
      console.log("WiFi Scan Result:", scanResults);
      setScanResults(scanResults); // Update state with scan results
    } catch (error) {
      console.error("Error parsing JSON:", error);
    }
  };

  // Function to send parameters (SSID and Password) to ESP32
  const sendParameters = async () => {
    if (!characteristic || !characteristic.control) {
      console.error("No control characteristic available. Connect to the device first.");
      return;
    }

    try {
      // Construct JSON object
      const data = { ssid, password };

      // Convert JSON to ArrayBuffer and send
      const encoder = new TextEncoder();
      const jsonStr = JSON.stringify(data);
      await characteristic.control.writeValue(encoder.encode(jsonStr));

      console.log("Parameters sent to ESP32:", data);
    } catch (error) {
      console.error("Error sending parameters:", error);
    }
  };

  // Render JSX
  return (
    <div>
      <h1>ESP32 Control</h1>
      <button type="button" onClick={connectToDevice}>
        Search Bluetooth Device
      </button>
      <button type="button" onClick={startWifiScan}>
        Start WiFi Scan
      </button>
      <form id="parameterForm">
        <label htmlFor="ssid">SSID:</label>
        <input type="text" id="ssid" onChange={(e) => setSSID(e.target.value)} value={ssid} />
        <label htmlFor="password">Password:</label>
        <input type="text" id="password" onChange={(e) => setPassword(e.target.value)} value={password} />
        <br />
        <button type="button" onClick={sendParameters}>
          Send Parameters
        </button>
      </form>
      {/* Display scan results */}
      <div>
        <h2>WiFi Scan Results:</h2>
        <ul>
          {scanResults.map((result, index) => (
            <li key={index}>
              <strong>SSID:</strong> {result.ssid}, <strong>RSSI:</strong> {result.rssi}
            </li>
          ))}
        </ul>
      </div>
    </div>
  );
};

export default AddDeviceForm;
