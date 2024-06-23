import { useEffect, useState } from "react";
import Axios from "axios";

import DeviceCard from "../components/DeviceCard";

const API_BASE_URL = import.meta.env.VITE_API_BASE_URL;

const Devices = () => {
  const [devices, setDevices] = useState([]);

  useEffect(() => {
    Axios.get(`${API_BASE_URL}/device`)
      .then((response) => {
        // handle success
        setDevices(response.data);
      })
      .catch((error) => {
        // handle error
        console.log(error);
      });
  }, []);

  return (
    <div>
      <section>
        <div className="devices">{devices && devices.map((device) => <DeviceCard key={device._id} device={device} />)}</div>
      </section>
    </div>
  );
};

export default Devices;
