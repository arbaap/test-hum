const mongoose = require("mongoose");

const riwayatpinSchema = new mongoose.Schema(
  {
    pin_lama: String,
    encryptedText2: String,
    pin_baru: String,
    encryptedText3: String,
  },
  {
    timestamps: true,
  }
);

const Data = mongoose.model("RiwayatPin", riwayatpinSchema);

module.exports = Data;