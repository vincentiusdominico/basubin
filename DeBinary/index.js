const express = require("express");
const expressLayouts = require("express-ejs-layouts");

require("./utils/database");

const Users = require("./Models/Customer");

const app = express();
const port = 3000;

// Home Page
app.get("/customers", (req, res) => {
  Users.find().then((users) => {
    res.send(users);
  });
});

app.listen(port, () => {
  console.log(`lisetinang at ${port}`);
});
