const express = require("express");
const cors = require("cors");
const morgan = require("morgan");
const dbConfig = require("./mongoDB");

const encryptedRoute = require("./routes/encryptedRoute");
const dataRoute = require("./routes/dataRoute");

const app = express();

app.use(cors());

app.use(express.json());

app.use(morgan("combined"));

app.use((req, res, next) => {
  if (
    req.header("x-forwarded-proto") !== "https" &&
    process.env.NODE_ENV === "production"
  ) {
    console.log(
      `Redirecting to HTTPS: https://${req.header("host")}${req.url}`
    );
    res.redirect(`https://${req.header("host")}${req.url}`);
  } else {
    next();
  }
});

app.get("/", async (req, res, next) => {
  return res.status(200).json({
    title: "Server Jalan",
    message: "Server siap digunakan!",
  });
});

app.use("/api/encrypted", encryptedRoute);
app.use("/api/data", dataRoute);

// const port = process.env.PORT || 5000;

// app.listen(port, () => console.log("Server Sensor is running on port", port));

const port = process.env.PORT || 5000;
const host = "0.0.0.0";

app.listen(port, host, () => {
  console.log(`Server Sensor is running on http://${host}:${port}`);
});
