import { useState, useEffect } from "react";

const AddDeviceForm = () => {
  const [bleState, setBleState] = useState("Disconnected");
  const [bleStateColor, setBleStateColor] = useState("#d13a30");
  const [valueContainer, setValueContainer] = useState("NaN");
  const [timestamp, setTimestamp] = useState("");
  const [valueSent, setValueSent] = useState("");
  const [networks, setNetworks] = useState([]);
  const [bleServer, setBleServer] = useState(null);
  const [bleServiceFound, setBleServiceFound] = useState(null);
  const [outputCharacteristicFound, setOutputCharacteristicFound] = useState(null);

  // Define BLE Device Specs
  const deviceName = "ESP32";
  const bleService = "19b10000-e8f2-537e-4f6c-d104768a1214";
  const inputCharacteristic = "19b10002-e8f2-537e-4f6c-d104768a1214";
  const outputCharacteristic = "19b10001-e8f2-537e-4f6c-d104768a1214";

  // Check if BLE is available in your Browser
  const isWebBluetoothEnabled = () => {
    if (!navigator.bluetooth) {
      console.log("Web Bluetooth API is not available in this browser!");
      setBleState("Web Bluetooth API is not available in this browser!");
      return false;
    }
    console.log("Web Bluetooth API supported in this browser.");
    return true;
  };

  // Connect to BLE Device and Enable Notifications
  const connectToDevice = () => {
    console.log("Initializing Bluetooth...");
    navigator.bluetooth
      .requestDevice({
        filters: [{ name: deviceName }],
        optionalServices: [bleService],
      })
      .then((device) => {
        console.log("Device Selected:", device.name);
        setBleState(`Connected`);
        setBleStateColor("#24af37");
        device.addEventListener("gattservicedisconnected", onDisconnected);
        return device.gatt.connect();
      })
      .then((gattServer) => {
        console.log("Connected to GATT Server");
        setBleServer(gattServer);
        return gattServer.getPrimaryService(bleService);
      })
      .then((service) => {
        console.log("Service discovered:", service.uuid);
        setBleServiceFound(service);
        return service.getCharacteristic(outputCharacteristic);
      })
      .then((characteristic) => {
        console.log("Characteristic discovered:", characteristic.uuid);
        setOutputCharacteristicFound(characteristic);
        characteristic.addEventListener("characteristicvaluechanged", handleCharacteristicChange);
        characteristic.startNotifications();
        console.log("Notifications Started.");
        return characteristic.readValue();
      })
      .then((value) => {
        const decodedValue = new TextDecoder().decode(value);
        console.log("Decoded value: ", decodedValue);
        setValueContainer(decodedValue);
      })
      .catch((error) => {
        console.log("Error: ", error);
      });
  };

  const onDisconnected = (event) => {
    console.log("Device Disconnected:", event.target.device.name);
    setBleState("Device Disconnected");
    setBleStateColor("#d13a30");
    connectToDevice();
  };

  const handleCharacteristicChange = (event) => {
    const newValueReceived = new TextDecoder().decode(event.target.value);
    console.log("Characteristic value changed: ", newValueReceived);
    setValueContainer(newValueReceived);

    if (newValueReceived.startsWith("[")) {
      try {
        const data = JSON.parse(newValueReceived);

        if (Array.isArray(data)) {
          const groupedNetworks = data.reduce((acc, network) => {
            const existingNetwork = acc.find((item) => item.ssid.toLowerCase() === network.ssid.toLowerCase());
            if (existingNetwork) {
              if (network.rssi < existingNetwork.rssi) {
                existingNetwork.rssi = network.rssi;
              }
            } else {
              acc.push(network);
            }
            return acc;
          }, []);

          setNetworks(groupedNetworks);
        } else {
          console.error("Parsed data is not an array:", data);
        }
      } catch (error) {
        console.error("Error parsing JSON:", error);
      }

      setTimestamp(getDateTime());
    }
  };

  const writeOnCharacteristic = (value) => {
    if (bleServer && bleServer.connected) {
      bleServiceFound
        .getCharacteristic(inputCharacteristic)
        .then((characteristic) => {
          console.log("Found the input characteristic: ", characteristic.uuid);
          const data = new Uint8Array([value]);
          return characteristic.writeValue(data);
        })
        .then(() => {
          setValueSent(value);
          console.log("Value written to input characteristic:", value);
        })
        .catch((error) => {
          console.error("Error writing to the input characteristic: ", error);
        });
    } else {
      console.error("Bluetooth is not connected. Cannot write to characteristic.");
      window.alert("Bluetooth is not connected. Cannot write to characteristic. \n Connect to BLE first!");
    }
  };

  const disconnectDevice = () => {
    console.log("Disconnect Device.");
    if (bleServer && bleServer.connected) {
      if (outputCharacteristicFound) {
        outputCharacteristicFound
          .stopNotifications()
          .then(() => {
            console.log("Notifications Stopped");
            return bleServer.disconnect();
          })
          .then(() => {
            console.log("Device Disconnected");
            setBleState("Device Disconnected");
            setBleStateColor("#d13a30");
          })
          .catch((error) => {
            console.log("An error occurred:", error);
          });
      } else {
        console.log("No characteristic found to disconnect.");
      }
    } else {
      console.error("Bluetooth is not connected.");
      window.alert("Bluetooth is not connected.");
    }
  };

  const getDateTime = () => {
    var currentdate = new Date();
    var day = ("00" + currentdate.getDate()).slice(-2);
    var month = ("00" + (currentdate.getMonth() + 1)).slice(-2);
    var year = currentdate.getFullYear();
    var hours = ("00" + currentdate.getHours()).slice(-2);
    var minutes = ("00" + currentdate.getMinutes()).slice(-2);
    var seconds = ("00" + currentdate.getSeconds()).slice(-2);

    var datetime = day + "/" + month + "/" + year + " at " + hours + ":" + minutes + ":" + seconds;
    return datetime;
  };

  const [showNetwork, setShowNetwork] = useState({});

  const toggleNetwork = (ssid) => {
    setShowNetwork((prevState) => ({
      ...prevState,
      [ssid]: !prevState[ssid],
    }));
  };

  const [showPasswords, setShowPasswords] = useState({});

  const togglePassword = (ssid) => {
    setShowPasswords((prevState) => ({
      ...prevState,
      [ssid]: !prevState[ssid],
    }));
  };

  const [password, setPassword] = useState({});

  const handlePassword = (ssid, value) => {
    setPassword((prevState) => ({
      ...prevState,
      [ssid]: value,
    }));
  };

  const connectToNetwork = async (ssid, password) => {
    try {
      if (!bleServer || !bleServer.connected || !bleServiceFound) {
        console.error("BLE server or service not connected. Connect to the device first.");
        return;
      }

      const inputCharacteristic = await bleServiceFound.getCharacteristic(inputCharacteristicUUID);
      if (!inputCharacteristic) {
        console.error("Input characteristic not found.");
        return;
      }

      const data = { ssid, password };
      const jsonStr = JSON.stringify(data);
      const dataArrayBuffer = new TextEncoder().encode(jsonStr);

      await inputCharacteristic.writeValue(dataArrayBuffer);
      console.log("Parameters sent to ESP32:", data);

      // Assuming there's a characteristic that sends responses back
      const responseCharacteristic = await bleServiceFound.getCharacteristic(responseCharacteristicUUID);
      if (!responseCharacteristic) {
        console.error("Response characteristic not found.");
        return;
      }

      // Read the response from the ESP32
      const responseValue = await responseCharacteristic.readValue();
      const responseText = new TextDecoder().decode(responseValue);

      // Check if the response indicates success
      if (responseText === "success") {
        setBleState("activated");
        console.log("ESP32 response: success, BLE state set to activated");
      } else {
        console.log("ESP32 response:", responseText);
      }
    } catch (error) {
      console.error("Error sending parameters or reading response:", error);
    }
  };

  return (
    <ol className="add-device-form">
      <button className="close" onClick={disconnectDevice}>
        <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24">
          <path stroke="currentColor" strokeLinecap="round" strokeWidth="2.125" d="M5 19L19 4.99996" />
          <path stroke="currentColor" strokeLinecap="round" strokeWidth="2.125" d="M19 19L4.99996 4.99996" />
        </svg>
      </button>
      <li className={`slide ${bleState === "Connected" ? "prev" : ""}`}>
        <h2>Add a Device</h2>
        <div className="image-container"></div>
        <button onClick={() => isWebBluetoothEnabled() && connectToDevice()}>Connect to BLE Device</button>
      </li>
      <li className={`slide ${bleState === "Connected" ? "" : "next"}`}>
        <h2>Add a Device</h2>{" "}
        <p>
          BLE state:{" "}
          <strong>
            <span style={{ color: bleStateColor }}>{bleState}</span>
          </strong>
        </p>
        <button onClick={() => writeOnCharacteristic(1)}>Scan Wifi</button>
        <ul className="networks">
          {networks.map((network, index) => (
            <li key={index}>
              <div className={`network ${showNetwork[network.ssid] ? "" : "hidden"}`}>
                <div className="ssid" onClick={() => toggleNetwork(network.ssid)}>
                  <span>
                    {network.ssid} {network.rssi}
                  </span>
                  <span className="rssi">
                    <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24">
                      <path stroke={network.rssi > -80 ? "rgba(var(--primary-text))" : "rgba( var(--gray-text))"} strokeLinecap="round" strokeWidth="2" d="M8 15.0618C10.3716 13.3128 13.6284 13.3129 16 15.0618" />
                      <path stroke={network.rssi > -60 ? "rgba(var(--primary-text))" : "rgba( var(--gray-text))"} strokeLinecap="round" strokeWidth="2" d="M5 12.0137C9.05353 8.66204 14.9465 8.66206 19 12.0137" />
                      <path stroke={network.rssi > -40 ? "rgba(var(--primary-text))" : "rgba( var(--gray-text))"} strokeLinecap="round" strokeWidth="2" d="M1.81079 8.81885C7.63896 3.72475 16.3713 3.72698 22.1969 8.82553" />
                      <path stroke={network.rssi > -100 ? "rgba(var(--primary-text))" : "rgba( var(--gray-text))"} strokeLinecap="round" strokeWidth="2.5" d="M12 18.75V18.8125" />
                    </svg>
                  </span>
                </div>
                <div className="password-container">
                  <div className="password">
                    <input type={`${showPasswords[network.ssid] ? "text" : "password"}`} onChange={(e) => handlePassword(network.ssid, e.target.value)} placeholder="Password" />
                    <div className="toogle" onClick={() => togglePassword(network.ssid)} onMouseEnter={() => togglePassword(network.ssid, true)} onMouseLeave={() => togglePassword(network.ssid, false)}>
                      <svg className={`${showPasswords[network.ssid] ? "hidden" : ""}`} xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24">
                        <circle cx="12" cy="12" r="3" stroke="currentColor" strokeWidth="2" />
                        <path
                          stroke="currentColor"
                          strokeWidth="2"
                          d="M2.56282 10.4308C1.88126 11.3662 1.88126 12.6338 2.56282 13.5692C5.31769 17.3503 8.46341 19.5 12 19.5C15.5366 19.5 18.6823 17.3503 21.4372 13.5692C22.1187 12.6338 22.1187 11.3662 21.4372 10.4308C18.6823 6.64972 15.5366 4.5 12 4.5C8.46342 4.5 5.31769 6.64972 2.56282 10.4308Z"
                        />
                      </svg>
                      <svg className={`${showPasswords[network.ssid] ? "" : "hidden"}`} xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24">
                        <path stroke="currentColor" strokeLinecap="round" strokeWidth="2" d="M3 3L21 21" />
                        <path
                          stroke="currentColor"
                          strokeLinecap="round"
                          strokeWidth="2"
                          d="M6.375 6.46875C5.03035 7.45323 3.7595 8.78825 2.56245 10.4313C1.88091 11.3667 1.88126 12.6338 2.56282 13.5692C5.31769 17.3503 8.46341 19.5 12 19.5C13.9648 19.5 15.8089 18.8365 17.5324 17.5984M20.4436 14.8412C20.7804 14.4404 21.1118 14.0159 21.4377 13.5685C22.1192 12.6331 22.1187 11.3662 21.4372 10.4308C18.6823 6.64972 15.5366 4.5 12 4.5C11.6986 4.5 11.4001 4.51561 11.1045 4.54651"
                        />
                      </svg>
                    </div>
                  </div>
                  <button onClick={() => connectToNetwork(network.ssid, password[network.ssid])}> Connect</button>
                </div>
              </div>
            </li>
          ))}
        </ul>
      </li>
      <li className={`slide ${bleState === "Connectedf" ? "" : bleState === "activated" ? "prev" : "next"}`}>
        <button>Go to Device</button>
      </li>
    </ol>
  );
};

export default AddDeviceForm;
