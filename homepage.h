String homePagePart1 = F(R"=====(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta http-equiv="refresh" content="5">
<title>Smart Steering Wheel</title>
<style>
    body {
        font-family: Arial, sans-serif;
        margin: 0;
        padding: 0;
    }
    header {
        background-color: #333;
        color: #fff;
        padding: 20px;
        text-align: center;
    }
    nav {
        background-color: #333;
        color: #fff;
        text-align: center;
        padding: 10px;
    }
    nav ul {
        list-style-type: none;
        padding: 0;
    }
    nav ul li {
        display: inline;
        margin: 0 10px;
    }
    nav ul li a {
        text-decoration: none;
        color: #fff;
        font-size: 18px;
    }
    .content {
        padding: 20px;
    }
    .section {
        margin-bottom: 30px;
    }
    .section h2 {
        color: #333;
    }
    .section p {
        color: #666;
    }
    footer {
        background-color: #333;
        color: #fff;
        text-align: center;
        padding: 20px;
        position: fixed;
        bottom: 0;
        width: 100%;
    }
</style>
</head>
<body>

<header>
    <h1>Smart Steering Wheel</h1>
</header>

<nav>
    <ul>
        <li><a href="#live-data">Live Data</a></li>
        <li><a href="#cloud-data">Cloud Data</a></li>
        <li><a href="#about-us">About Us</a></li>
        <li><a href="sdg">Goals</a></li>
    </ul>
</nav>
<div style="text-align:center">
<div class="content">
    <div id="live-data" class="section">
        <h2>Live Data</h2>
        <p>This section displays live data from the smart steering wheel.</p>
        <h2 style="color: #007bff;">Current Sensor Data</h2>
                    <p style="margin-bottom: 5px;">Temperature:)=====");String homePagePart2 = F(R"=====( °C</p>
                    <p style="margin-bottom: 5px;">Humidity:)=====");String homePagePart3 = F(R"=====( %</p>
                    <p style="margin-bottom: 5px;">Speed:)=====");String homePagePart4 = F(R"=====( m/s²</p>
                    <p style="margin-bottom: 5px;">BPM:)=====");String homePagePart5 = F(R"=====(</p>
                    <p style="margin-bottom: 5px;">Blood Oxygen:)=====");String homePagePart6 = F(R"=====( %</p>
    </div>

    <div id="cloud-data" class="section">
        <h2>Cloud Data</h2>
        <p>This section shows data stored in the cloud related to the smart steering wheel.</p>
    </div>

    <div id="about-us" class="section">
        <h2>About Us</h2>
        <p>This section provides information about the smart steering wheel and its team.</p>
    </div>
    
    <div id="sdg" class="section">
        <h2>Sustainable Development Goals</h2>
        <p>This section provides information about the smart steering wheel and its team.</p>
    </div>
</div>

<footer>
    <p>&copy; 2024 Smart Steering Wheel</p>
</footer>

</body>
</html>
