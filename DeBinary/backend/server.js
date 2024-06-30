require("dotenv").config();
const express = require("express");
const cors = require("cors");
const expressLayouts = require("express-ejs-layouts");

const userRoutes = require("./routes/users");
const deviceRoutes = require("./routes/devices");

const app = express();
app.use(cors());

const PORT = process.env.PORT;
const URI = process.env.MONGO_URI;

const mongoose = require("mongoose");
const dotenv = require("dotenv");

app.get("/", (req, res) => {
  res.send("Hello World");
});

mongoose
  .connect(URI)
  .then(() => {
    console.log("Connected Successfully");
    app.listen(process.env.PORT, () => {
      console.log(`lisetinang at ${process.env.PORT}`);
    });

    // middleware
    app.use(express.json());

    app.use((req, res, next) => {
      console.log(req.path, req.method);
      next();
    });

    // routes
    app.get("/", (req, res) => {
      res.json("yup");
    });

    app.use("/api/user", userRoutes);
    app.use("/api/device", deviceRoutes);
  })
  .catch((error) => console.log(error));
