const express = require("express");
const router = express.Router();
const Encrypted = require("../models/dbEncrypted");

router.post("/kirimEncrypted", async (req, res) => {
  const { encrypted } = req.body;

  const timestamp = new Date();

  const newEncrtypted = new Encrypted({
    encryptedText: encrypted,
  });

  try {
    const result = await newEncrtypted.save();
    console.log("Berhasil menyimpan data enkripsi test:", result);
    res.status(200).json({ message: "Berhasil menyimpan data enkripsi test" });
  } catch (err) {
    console.log("Gagal menyimpan data enkripsi:", err);
    res.status(500).json({ message: "Gagal menyimpan data enkripsi" });
  }
});

router.get("/getDataEncrypted", async (req, res) => {
  try {
    const data = await Encrypted.find({});
    console.log("Berhasil mengambil data enkripsi");
    res.status(200).json(data);
  } catch (err) {
    console.log("Gagal mengambil data enkripsi:", err);
    res.status(500).json({ message: "Gagal mengambil data enkripsi" });
  }
});

module.exports = router;
