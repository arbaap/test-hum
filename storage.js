const express = require("express");
const cors = require("cors");

const dbConfig = require("./mongoDB");

const encryptedRoute = require("./routes/encryptedRoute");
const dataRoute = require("./routes/dataRoute");
const riwayatPinRoute = require("./routes/riwayatPinRoute");


const app = express();

app.use(cors());

app.use(express.json());

app.get("/", async (req, res, next) => {
  return res.status(200).json({
    title: "Server Jalan",
    message: "Server siap digunakan!",
  });
});

app.use("/api/encrypted", encryptedRoute);
app.use("/api/data", dataRoute);
app.use("/api/riwayatpin", riwayatPinRoute);


const port = process.env.PORT || 3000;

app.listen(port, () => console.log("Server Sensor is running on port", port));
