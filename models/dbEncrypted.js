const mongoose = require("mongoose");

const encryptedSchema = new mongoose.Schema(
  {
    encryptedText: String,
  },
  {
    timestamps: true,
  }
);

const Encrypted = mongoose.model("Encrypted", encryptedSchema);

module.exports = Encrypted;
