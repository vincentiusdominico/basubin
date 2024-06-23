const Device = require("../Models/Device");
const mongoose = require("mongoose");

// get all Devices
const getDevices = async (req, res) => {
  try {
    const devices = await Device.find({}).sort({ createdAt: -1 });
    res.status(200).json(devices);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
};

// get a single Device
const getDevice = async (req, res) => {
  const { id } = req.params;

  if (!mongoose.Types.ObjectId.isValid(id)) {
    return res.status(404).json({ error: "Invalid device ID" });
  }

  try {
    const device = await Device.findById(id);

    if (!device) {
      return res.status(404).json({ error: "Device not found" });
    }

    res.status(200).json(device);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
};

// create new Device
const createDevice = async (req, res) => {
  const { ip, name, model, isOpen, isLock, capacity, temperature, humidity, running, error, connection, lastActiveAt } = req.body;

  try {
    const device = await Device.create({ ip, name, model, isOpen, isLock, capacity, temperature, humidity, running, error, connection, lastActiveAt });
    res.status(201).json(device);
  } catch (error) {
    res.status(400).json({ error: error.message });
  }
};

// update a Device
const updateDevice = async (req, res) => {
  const { id } = req.params;

  if (!mongoose.Types.ObjectId.isValid(id)) {
    return res.status(404).json({ error: "Invalid device ID" });
  }

  try {
    const device = await Device.findOneAndUpdate({ _id: id }, { ...req.body }, { new: true });

    if (!device) {
      return res.status(404).json({ error: "Device not found" });
    }

    res.status(200).json(device);
  } catch (error) {
    res.status(400).json({ error: error.message });
  }
};

// delete a Device
const deleteDevice = async (req, res) => {
  const { id } = req.params;

  if (!mongoose.Types.ObjectId.isValid(id)) {
    return res.status(404).json({ error: "Invalid device ID" });
  }

  try {
    const device = await Device.findOneAndDelete({ _id: id });

    if (!device) {
      return res.status(404).json({ error: "Device not found" });
    }

    res.status(200).json(device);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
};

module.exports = { getDevices, getDevice, createDevice, updateDevice, deleteDevice };
