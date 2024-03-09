const mongoose = require("mongoose");

const dataSchema = new mongoose.Schema(
  {
    status_pintu: String,
    teks_asli: String,
    teks_enkripsi: String,
    teks_dekripsi: String,
    pin_lama: String,
    teks_enkripsi_sebelum: String,
    pin_baru: String,
    teks_enkripsi_sesudah: String,
  },
  {
    timestamps: true,
  }
);

const Data = mongoose.model("Data", dataSchema);

module.exports = Data;
