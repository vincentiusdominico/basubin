const mongoose = require("mongoose");
const dotenv = require("dotenv");

dotenv.config();

const PORT = process.env.PORT || 7000;
const URI = process.env.MONGO_URI;
mongoose
  .connect(URI)
  .then(() => {
    console.log("Connected Successfully");
  })
  .catch((error) => console.log(error));
