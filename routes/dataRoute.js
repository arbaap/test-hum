const express = require("express");
const router = express.Router();
const Data = require("../models/dbData");
const { body, validationResult } = require("express-validator");

router.post(
  "/kirimData",
  [
    body("kelembapan_tanah").notEmpty().isNumeric(),
    body("temperature").notEmpty().isNumeric(),
    body("humidity").notEmpty().isNumeric(),
    body("pH_tanah").notEmpty().isNumeric(),
  ],
  async (req, res) => {
    const errors = validationResult(req);
    if (!errors.isEmpty()) {
      return res.status(400).json({ errors: errors.array() });
    }

    const { kelembapan_tanah, temperature, humidity, pH_tanah } = req.body;
    const clientIp =
      req.headers["x-forwarded-for"] || req.connection.remoteAddress;
    const timestamp = new Date();

    const newData = new Data({
      kelembapan_tanah,
      temperature,
      humidity,
      pH_tanah,
    });

    try {
      const result = await newData.save();
      console.log(`Berhasil menyimpan data semua dari IP: ${clientIp}`, result);
      res.status(200).json({
        message: "Berhasil menyimpan data semua",
        ip: clientIp,
      });
    } catch (err) {
      console.log(`Gagal menyimpan data semua dari IP: ${clientIp}`, err);
      res.status(500).json({
        message: "Gagal menyimpan data semua",
        ip: clientIp,
        error: err.message,
      });
    }
  }
);

router.get("/getDataAll", async (req, res) => {
  const clientIp =
    req.headers["x-forwarded-for"] || req.connection.remoteAddress;

  try {
    const data = await Data.find({}).sort({ createdAt: -1 });
    console.log(`Berhasil mengambil data all dari IP: ${clientIp}`);
    res.status(200).json(data);
  } catch (err) {
    console.log(`Gagal mengambil data all dari IP: ${clientIp}`, err);
    res.status(500).json({
      message: "Gagal mengambil data all",
      ip: clientIp,
    });
  }
});

module.exports = router;
