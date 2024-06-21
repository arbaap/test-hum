const express = require("express");
const router = express.Router();
const Data = require("../models/dbData");
const axios = require("axios");

router.post("/kirimData", async (req, res) => {
  const {
    kelembapan_tanah,
    temperature,
    humidity,
    pH_tanah,
 } = req.body;

  const timestamp = new Date();

  const newData = new Data({
    kelembapan_tanah: kelembapan_tanah,
    temperature: temperature,
    humidity: humidity,
    pH_tanah: pH_tanah,
  });

  try {
    const result = await newData.save();
    console.log("Berhasil menyimpan data semua:", result);
    res.status(200).json({ message: "Berhasil menyimpan data semua" });
  } catch (err) {
    console.log("Gagal menyimpan data semua:", err);
    res.status(500).json({ message: "Gagal menyimpan data semua" });
  }
});


router.post("/kirimDatas", async (req, res) => {
  const { kelembapan_tanah, temperature, humidity, pH_tanah } = req.body;

  const timestamp = new Date();

  const newData = new Data({
    kelembapan_tanah: kelembapan_tanah,
    temperature: temperature,
    humidity: humidity,
    pH_tanah: pH_tanah,
  });

  try {
    const result = await newData.save();
    console.log("Berhasil menyimpan data:", result);

    const serverHttpUrl = "http://test-hum.vercel.app/api/data/kirimDatas";
    const serverHttpsUrl = "https://test-hum.vercel.app/api/data/kirimDatas";

    const payload = {
      kelembapan_tanah,
      temperature,
      humidity,
      pH_tanah,
    };

    const response = await axios.post(serverHttpUrl, payload);

    if (response.status === 301 || response.status === 302) {
      console.log("Menerima redirect, mengirim ulang dengan HTTPS...");
      const httpsResponse = await axios.post(serverHttpsUrl, payload);
      res.status(httpsResponse.status).json(httpsResponse.data);
    } else {
      console.log("Data terkirim via HTTP");
      res.status(response.status).json(response.data);
    }

    console.log(response);
  } catch (err) {
    console.error("Gagal menyimpan data atau HTTP error:", err.message);
    res
      .status(500)
      .json({ message: "Gagal menyimpan data atau terjadi kesalahan HTTP" });
  }
});

router.get("/getDataAll", async (req, res) => {
  try {
    const data = await Data.find({}).sort({ createdAt: -1 });
    console.log("Berhasil mengambil data all");
    res.status(200).json(data);
  } catch (err) {
    console.log("Gagal mengambil data all:", err);
    res.status(500).json({ message: "Gagal mengambil data all" });
  }
});

module.exports = router;
