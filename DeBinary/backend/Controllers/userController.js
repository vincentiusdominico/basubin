const User = require("../Models/User");
const mongoose = require("mongoose");

// get all user
const getUsers = async (req, res) => {
  const users = await User.find({}).sort({ createdAt: -1 });

  res.status(200).json(users);
};

// get single user
const getUser = async (req, res) => {
  const { id } = req.params;

  if (!mongoose.Types.ObjectId.isValid(id)) {
    return res.status(404).json({ error: error.message });
  }

  const user = await User.findById(id);

  if (!customer) {
    return res.status(404).json({ error: error.message });
  }

  res.status(200).json(user);
};

// create new user
const createUser = async (req, res) => {
  const { name, address, contact, member } = req.body;

  try {
    const user = await User.create({ name, address, contact, member });
    res.status(201).json(user);
  } catch (error) {
    res.status(400).json({ error: error.message });
  }
};

// update new user
const updateUser = async (req, res) => {
  const { id } = req.params;

  if (!mongoose.Types.ObjectId.isValid(id)) {
    return res.status(404).json({ error: error.message });
  }

  const { name } = req.body;
  const user = await User.findOneAndUpdate(
    { _id: id },
    {
      ...req.body,
    }
  );

  if (!customer) {
    return res.status(404).json({ error: error.message });
  }

  res.status(200).json(user);
};

// delete a user
const deleteUser = async (req, res) => {
  const { id } = req.params;

  if (!mongoose.Types.ObjectId.isValid(id)) {
    return res.status(404).json({ error: error.message });
  }

  const user = await User.findOneAndDelete({ _id: id });

  if (!customer) {
    return res.status(404).json({ error: error.message });
  }

  res.status(200).json(user);
};

module.exports = { getUsers, getUser, createUser, updateUser, deleteUser };
