const mongoose = require("mongoose");

const dataSchema = new mongoose.Schema(
  {
    kelembapan_tanah: String,
    temperature: String,
    humidity: String,
    pH_tanah: String,
  },
  {
    timestamps: true,
  }
);

const Data = mongoose.model("Data", dataSchema);

module.exports = Data;
