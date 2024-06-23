const mongoose = require("mongoose");

const schema = new mongoose.Schema(
  {
    ip: {
      type: String,
      required: false,
    },
    name: {
      type: String,
      required: false,
    },
    model: {
      type: String,
      required: false,
    },
    isOpen: {
      type: Boolean,
      required: false,
    },
    isLock: {
      type: Boolean,
      required: false,
    },
    capacity: {
      type: Number,
      required: false,
    },
    temperature: {
      type: Number,
      required: false,
    },
    humidity: {
      type: Number,
      required: false,
    },
    running: {
      process: {
        type: String,
        required: false,
      },
      startedAt: {
        type: Date,
        required: false,
      },
      finishedAt: {
        type: Date,
        required: false,
      },
    },
    error: {
      message: {
        type: String,
        required: false,
      },
      createdAt: {
        type: Date,
        required: false,
      },
    },
    connection: {
      ssid: {
        type: String,
        required: false,
      },
      strength: {
        type: Number,
        required: false,
      },
    },
    lastActiveAt: {
      type: Date,
      required: false,
    },
  },
  { timestamps: true }
);

module.exports = mongoose.model("Device", schema);
