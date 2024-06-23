const express = require("express");
const { getUsers, getUser, createUser, updateUser, deleteUser } = require("../Controllers/userController");

const router = express.Router();

// GET all Users
router.get("/", getUsers);

// GET a single cutomer
router.get("/:id", getUser);

// POST a new User
router.post("/", createUser);

// UPDATE a User
router.patch("/:id", updateUser);

// DELETE a User
router.delete("/:id", deleteUser);

module.exports = router;
