import DeviceModel from "./DeviceModel.jsx";
const DeviceCard = ({ device }) => {
  return (
    <div className="device-card">
      <div className="container">
        <span className="dot"></span>
        <div className="image-container">
          <DeviceModel />
        </div>
        <div className="text-container">
          <h2>{device.name}</h2>
          <p>{device.model}</p>
        </div>
      </div>
    </div>
  );
};

export default DeviceCard;
