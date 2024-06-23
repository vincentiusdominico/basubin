const express = require("express");
const { getDevices, getDevice, createDevice, updateDevice, deleteDevice } = require("../Controllers/deviceController");

const router = express.Router();

// GET all Devices
router.get("/", getDevices);

// GET a single cutomer
router.get("/:id", getDevice);

// POST a new Device
router.post("/", createDevice);

// UPDATE a Device
router.patch("/:id", updateDevice);

// DELETE a Device
router.delete("/:id", deleteDevice);

module.exports = router;
