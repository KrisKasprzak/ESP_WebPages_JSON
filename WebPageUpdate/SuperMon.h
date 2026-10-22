/*


  OK, ya ready for some fun? HTML + CSS styling + javascript all in an un-debuggable environment

  NOTE (1)
  the max size of esp strings is default to 64K, the page is > 64K, current need is around 85K. 
  If edits are made and web page fails to load, you are probably
  over the limit. turn debug on and see size of page. To increase the page size
  C:\Users\Administrator\AppData\Local\Arduino15\packages\esp32\hardware\esp32\xxxxx...
  \cores\esp32\WString.h
  around line 335 change
  CAPACITY_MAX =85000
  //CAPACITY_MAX = 65535

  one trick I've learned to how to debug HTML and CSS code.

  get all your HTML code (from html to /html) and past it into this test site
  muck with the HTML and CSS code until it's what you want
  https://www.w3schools.com/html/tryit.asp?filename=tryhtml_intro

  you can also save the HTML to a .html file on your PC and open it in a browser. Press F12 to open the
  browser's developer tools, where you can tweak CSS live and see javascript errors in the Console tab
  (the data won't update since there is no ESP to talk to, but the layout will show)

  I'm using class designators to set styles and id's for data updating
  for example:
  the CSS class .value defines what the number will look like
  <div class="value" id="b0">--</div>

  the JSON data will update the data where id = "b0"
  java script then uses getElementById
  document.getElementById("b0").innerHTML=data.B0;

  .. you can also swap classes from javascript to change the look, here's what that looks like
  <div class="status" id="switch">--</div>

  java script then sets className to pick a different style (.status.on or .status.off in the CSS)
  document.getElementById("switch").className="status on";

  this keeps colors and styles in the CSS where they belong instead of scattered through the javascript

  the main general sections of a web page are the following and used here

  <html>
    <head>
    // page title, settings, and CSS style stuff in here
    <style>
    // dump CSS style stuff in here
    </style>
    </head>
    <body>
      <header>
      // put header code for cute banners here
      </header>
      <main>
      // the buld of your web page contents
      </main>
      <footer>
      // put cute footer (c) 2021 xyz inc type thing
      </footer>
    </body>
    <script>
    // you java code between these tags
    </script>
  </html>

  the page layout uses "cards" (white boxes with rounded corners and a soft shadow) placed in a CSS grid
  the grid automatically wraps the cards into fewer columns on a phone, so the same page works on
  a PC, tablet, or smartphone. The page also follows the device's light/dark mode setting


*/

// note R"KEYWORD( html page code )KEYWORD";
// again I hate strings, so char is it and this method let's us write naturally

const char PAGE_MAIN[] PROGMEM = R"=====(

<!DOCTYPE html>
<html lang="en">

<head>
  <meta charset="utf-8">
  <!-- this line makes the page scale correctly on phones instead of showing a tiny desktop page -->
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Sensor Monitor</title>

  <style>
    /* colors are defined once here as CSS variables, change them here to restyle the whole page */
    :root {
      --bg: #f1f5f9;
      --card: #ffffff;
      --text: #0f172a;
      --muted: #64748b;
      --border: #e2e8f0;
      --track: #e2e8f0;
      --accent: #2563eb;
      --high: #dc2626;
      --on: #16a34a;
      --off: #64748b;
      --shadow: 0 1px 3px rgba(15, 23, 42, 0.08), 0 4px 12px rgba(15, 23, 42, 0.06);
    }
    /* dark mode colors, used automatically when the phone or PC is set to dark mode */
    @media (prefers-color-scheme: dark) {
      :root {
        --bg: #0f172a;
        --card: #1e293b;
        --text: #e2e8f0;
        --muted: #94a3b8;
        --border: #334155;
        --track: #334155;
        --shadow: 0 1px 3px rgba(0, 0, 0, 0.4);
      }
    }
    * {
      box-sizing: border-box;
    }
    body {
      margin: 0;
      background: var(--bg);
      color: var(--text);
      /* system-ui uses the device's native font (Segoe UI on Windows, San Francisco on Apple, Roboto on Android) */
      font-family: system-ui, -apple-system, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
      line-height: 1.4;
    }

    /* header bar that stays at the top of the page */
    .topbar {
      position: sticky;
      top: 0;
      z-index: 10;
      background: #1e3a8a;
      color: #ffffff;
      box-shadow: 0 2px 8px rgba(0, 0, 0, 0.2);
    }
    .topbar-inner {
      max-width: 1000px;
      margin: 0 auto;
      padding: 12px 20px;
      display: flex;
      align-items: center;
      justify-content: space-between;
      flex-wrap: wrap;
      gap: 8px 24px;
    }
    .topbar h1 {
      margin: 0;
      font-size: 24px;
      font-weight: 600;
    }
    .clock {
      display: flex;
      gap: 24px;
      font-variant-numeric: tabular-nums;
    }
    .clock .label {
      display: block;
      font-size: 11px;
      text-transform: uppercase;
      letter-spacing: 0.08em;
      opacity: 0.7;
    }
    .clock span:not(.label) {
      font-size: 16px;
      font-weight: 600;
    }

    /* main content area */
    .container {
      max-width: 1000px;
      margin: 0 auto;
      padding: 8px 20px 24px 20px;
    }
    .section-title {
      margin: 24px 0 12px 0;
      font-size: 14px;
      font-weight: 600;
      text-transform: uppercase;
      letter-spacing: 0.08em;
      color: var(--muted);
    }

    /* the grid fits as many 260px wide cards per row as the screen allows */
    .grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(260px, 1fr));
      gap: 16px;
    }
    .card {
      background: var(--card);
      border: 1px solid var(--border);
      border-radius: 12px;
      box-shadow: var(--shadow);
      padding: 18px 20px;
    }
    /* a wide card spans the whole row */
    .card.wide {
      grid-column: 1 / -1;
    }
    .card-head {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 12px;
    }
    .card-title {
      font-size: 16px;
      font-weight: 600;
    }
    .tag {
      font-size: 12px;
      color: var(--muted);
      background: var(--bg);
      border-radius: 999px;
      padding: 2px 10px;
    }

    /* sensor readings */
    .readings {
      display: flex;
      gap: 32px;
      margin-bottom: 14px;
    }
    .value {
      font-size: 32px;
      font-weight: 700;
      /* tabular-nums keeps every digit the same width so the numbers don't jiggle as they change */
      font-variant-numeric: tabular-nums;
    }
    .unit {
      font-size: 13px;
      color: var(--muted);
    }

    /* bar graph showing the analog reading, javascript sets the width and color of .bar-fill */
    .bar {
      height: 10px;
      background: var(--track);
      border-radius: 999px;
      overflow: hidden;
    }
    .bar-fill {
      height: 100%;
      width: 0%;
      background: var(--accent);
      border-radius: 999px;
      transition: width 200ms ease-out, background-color 200ms;
    }

    /* switch status pill, javascript swaps between the .on and .off classes */
    .status {
      display: inline-block;
      padding: 6px 16px;
      border-radius: 999px;
      font-weight: 600;
      color: #ffffff;
      background: var(--off);
    }
    .status.on {
      background: var(--on);
    }
    .status.off {
      background: var(--off);
    }

    /* control buttons, the .on class is added by javascript when the output is on */
    .control-row {
      display: flex;
      justify-content: space-between;
      align-items: center;
      gap: 12px;
    }
    .btn {
      min-width: 110px;
      padding: 10px 20px;
      border: 2px solid var(--accent);
      border-radius: 8px;
      background: var(--accent);
      color: #ffffff;
      font-size: 15px;
      font-weight: 600;
      cursor: pointer;
      transition: background-color 150ms, color 150ms, transform 50ms;
    }
    .btn:hover {
      filter: brightness(1.1);
    }
    .btn:active {
      transform: scale(0.97);
    }
    .btn.on {
      background: transparent;
      color: var(--accent);
    }

    /* fan speed slider, accent-color styles the slider in modern browsers */
    .fanrpmslider {
      width: 100%;
      margin: 8px 0 4px 0;
      accent-color: var(--accent);
      cursor: pointer;
    }
    .rpm {
      font-size: 24px;
      font-weight: 700;
      font-variant-numeric: tabular-nums;
    }
    .scale {
      display: flex;
      justify-content: space-between;
      font-size: 12px;
      color: var(--muted);
    }

    .foot {
      text-align: center;
      font-size: 13px;
      color: var(--muted);
      padding: 16px;
    }

    /* ---------- phone layout ----------
       everything in this block only applies to screens 600px wide or less (phones held upright)
       the goal is to fit the whole page on one phone screen with no scrolling:
       - cards are placed 2 per row instead of 1
       - the header is a single short row
       - spacing and fonts are tightened up
       if you add more cards and the page no longer fits, shrink things here (desktop is not affected) */
    @media (max-width: 600px) {
      .topbar-inner {
        padding: 8px 12px;
        gap: 4px 12px;
        flex-wrap: nowrap;
      }
      .topbar h1 {
        font-size: 19px;
        white-space: nowrap;
      }
      .clock {
        gap: 12px;
      }
      /* the DATE and TIME labels are obvious from the values, so hide them to save space */
      .clock .label {
        display: none;
      }
      .clock span:not(.label) {
        font-size: 14px;
        white-space: nowrap;
      }
      .container {
        padding: 0 10px 4px 10px;
      }
      .section-title {
        margin: 12px 0 6px 2px;
        font-size: 12px;
      }
      /* 2 cards per row */
      .grid {
        grid-template-columns: 1fr 1fr;
        gap: 10px;
      }
      /* cards with this class span the whole row on a phone */
      .phone-wide {
        grid-column: 1 / -1;
      }
      .card {
        padding: 12px 14px;
        border-radius: 10px;
      }
      .card-head {
        margin-bottom: 6px;
        gap: 8px;
      }
      .card-title {
        font-size: 15px;
      }
      .tag {
        font-size: 11px;
        padding: 1px 8px;
      }
      .readings {
        gap: 16px;
        margin-bottom: 10px;
      }
      .value {
        font-size: 26px;
      }
      .unit {
        font-size: 12px;
      }
      .bar {
        height: 8px;
      }
      /* put the switch title and the ON/OFF pill on one line */
      .switch-card {
        display: flex;
        justify-content: space-between;
        align-items: center;
      }
      .switch-card .card-head {
        margin-bottom: 0;
      }
      .status {
        padding: 4px 14px;
        font-size: 14px;
      }
      /* stack the control title above a full width button so 2 controls fit side by side */
      .control-row {
        flex-direction: column;
        align-items: stretch;
        gap: 8px;
      }
      .btn {
        min-width: 0;
        padding: 10px;
      }
      .rpm {
        font-size: 20px;
      }
      .fanrpmslider {
        margin: 4px 0 2px 0;
      }
      .foot {
        padding: 8px;
        font-size: 11px;
      }
    }
  </style>
</head>

<body onload="process()">

  <header class="topbar">
    <div class="topbar-inner">
      <h1>Sensor Monitor</h1>
      <div class="clock">
        <div><span class="label">Date</span><span id="date">--/--/----</span></div>
        <div><span class="label">Time</span><span id="time">--:--:--</span></div>
      </div>
    </div>
  </header>

  <main class="container">

    <h2 class="section-title">Sensor Readings</h2>
    <div class="grid">

      <section class="card">
        <div class="card-head">
          <span class="card-title">Analog Pin 33</span>
          <span class="tag">A0</span>
        </div>
        <div class="readings">
          <div><div class="value" id="b0">--</div><div class="unit">bits</div></div>
          <div><div class="value" id="v0">--</div><div class="unit">volts</div></div>
        </div>
        <div class="bar"><div class="bar-fill" id="bar0"></div></div>
      </section>

      <section class="card">
        <div class="card-head">
          <span class="card-title">Analog Pin 32</span>
          <span class="tag">A1</span>
        </div>
        <div class="readings">
          <div><div class="value" id="b1">--</div><div class="unit">bits</div></div>
          <div><div class="value" id="v1">--</div><div class="unit">volts</div></div>
        </div>
        <div class="bar"><div class="bar-fill" id="bar1"></div></div>
      </section>

      <section class="card phone-wide switch-card">
        <div class="card-head">
          <span class="card-title">Digital Switch</span>
          <span class="tag">Pin 26</span>
        </div>
        <div class="status" id="switch">--</div>
      </section>

    </div>

    <h2 class="section-title">Sensor Controls</h2>
    <div class="grid">

      <section class="card">
        <div class="control-row">
          <span class="card-title">On-board LED</span>
          <button type="button" class="btn" id="btn0" onclick="ButtonPress0()">Toggle</button>
        </div>
      </section>

      <section class="card">
        <div class="control-row">
          <span class="card-title">Switch</span>
          <button type="button" class="btn" id="btn1" onclick="ButtonPress1()">Toggle</button>
        </div>
      </section>

      <section class="card wide">
        <div class="card-head">
          <span class="card-title">Fan Speed Control</span>
          <span><span class="rpm" id="fanrpm">0</span> <span class="unit">RPM</span></span>
        </div>
        <input type="range" class="fanrpmslider" min="0" max="255" value="0" oninput="UpdateSlider(this.value)">
        <div class="scale"><span>Off</span><span>Full speed</span></div>
      </section>

    </div>
  </main>

  <footer class="foot">ESP32 Web Page Creation and Data Update Demo</footer>

  <script type = "text/javascript">

    // colors for the bar graphs, blue for normal readings and red for high readings
    var NORMAL_COLOR = "#2563eb";
    var HIGH_COLOR = "#dc2626";

    // global variable visible to all java functions
    // note: despite its name, XMLHttpRequest can request any kind of text data, including JSON
    var jsonHttp=createHttpObject();

    // function to create the object used to request the JSON data from the ESP
    function createHttpObject(){
      var http;
      if(window.XMLHttpRequest){
        http=new XMLHttpRequest();
      }
      else{
        http=new ActiveXObject("Microsoft.XMLHTTP");
      }
      return http;
    }

    // function to handle button press from HTML code above
    // and send a processing string back to server
    // this processing string is use in the .on method
    function ButtonPress0() {
      var xhttp = new XMLHttpRequest();
      var message;
      // if you want to handle an immediate reply (like status from the ESP
      // handling of the button press use this code
      // since this button status from the ESP is in the main JSON data
      // we don't need this
      // remember that if you want immediate processing feedbac you must send it
      // in the ESP handling function and here
      /*
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          message = this.responseText;
          // update some HTML data
        }
      }
      */

      // true = asynchronous, the page does not freeze while waiting for the ESP to reply
      // (browsers now warn about synchronous requests)
      xhttp.open("PUT", "BUTTON_0", true);
      xhttp.send();
    }


    // function to handle button press from HTML code above
    // and send a processing string back to server
    // this processing string is use in the .on method
    function ButtonPress1() {
      var xhttp = new XMLHttpRequest();
      /*
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("button1").innerHTML = this.responseText;
        }
      }
      */
      xhttp.open("PUT", "BUTTON_1", true);
      xhttp.send();
    }

    function UpdateSlider(value) {
      var xhttp = new XMLHttpRequest();
      // this time i want immediate feedback to the fan speed
      // yea yea yea i realize i'm computing fan speed but the point
      // is how to give immediate feedback
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // update the web based on reply from  ESP
          document.getElementById("fanrpm").innerHTML=this.responseText;
        }
      }
      // this syntax is really weird the ? is a delimiter
      // first arg UPDATE_SLIDER is use in the .on method
      // server.on("/UPDATE_SLIDER", UpdateSlider);
      // then the second arg VALUE is use in the processing function
      // String t_state = server.arg("VALUE");
      // then + the controls value property
      xhttp.open("PUT", "UPDATE_SLIDER?VALUE="+value, true);
      xhttp.send();
    }

    // updates one analog sensor card: the bits, the volts, and the bar graph
    // bitsId, voltsId, and barId are the id's of the HTML elements to update
    function UpdateSensor(bitsId, voltsId, barId, bits, volts) {

      // 12 bit reads go from 0 to 4095, so divide by 40.95 to get 0 to 100%
      var barwidth = bits / 40.95;

      document.getElementById(bitsId).innerHTML=bits;

      // JSON numbers arrive as real numbers, so format to 1 decimal place for display
      // (for example 1 would otherwise show as "1" instead of "1.0")
      document.getElementById(voltsId).innerHTML=Number(volts).toFixed(1);

      // set the bar length, and color it red above half scale, blue below
      document.getElementById(barId).style.width=(barwidth+"%");
      document.getElementById(barId).style.backgroundColor=(bits > 2048) ? HIGH_COLOR : NORMAL_COLOR;
    }

    // function to handle the response from the ESP
    // the ESP sends JSON that looks like this:
    // {"B0":1234,"V0":1.0,"B1":2345,"V1":1.9,"LED":false,"SWITCH":true}
    // JSON CHANGES
    function response(){
      var data;
      var dt = new Date();

      // wait until the request is complete (readyState 4) and successful (status 200)
      // this function is called several times during a request, so ignore the in-between states
      if(this.readyState != 4 || this.status != 200){
        return;
      }

      // convert the JSON text from the ESP into a javascript object
      // if the text is not valid JSON (for example a truncated reply), skip this update
      try {
        data = JSON.parse(this.responseText);
      }
      catch (e) {
        console.log("bad JSON from ESP: " + this.responseText);
        return;
      }

      // get host date and time
      document.getElementById("time").innerHTML = dt.toLocaleTimeString();
      document.getElementById("date").innerHTML = dt.toLocaleDateString();

      // the JSON data is now accessed by name, for example data.B0
      // no more digging through XML tags with getElementsByTagName

      // A0 and A1
      UpdateSensor("b0", "v0", "bar0", data.B0, data.V0);
      UpdateSensor("b1", "v1", "bar1", data.B1, data.V1);

      // LED status is sent as a JSON true/false, so it can be tested directly
      // the button shows what pressing it will do, and the .on class changes its look
      if (!data.LED){
        document.getElementById("btn0").innerHTML="Turn ON";
        document.getElementById("btn0").className="btn";
      }
      else{
        document.getElementById("btn0").innerHTML="Turn OFF";
        document.getElementById("btn0").className="btn on";
      }

      // switch status is also a JSON true/false
      // the colors for the status pill are in the .status.on and .status.off CSS classes
      if (!data.SWITCH){
        document.getElementById("switch").innerHTML="Switch is OFF";
        document.getElementById("switch").className="status off";
        document.getElementById("btn1").innerHTML="Turn ON";
        document.getElementById("btn1").className="btn";
      }
      else {
        document.getElementById("switch").innerHTML="Switch is ON";
        document.getElementById("switch").className="status on";
        document.getElementById("btn1").innerHTML="Turn OFF";
        document.getElementById("btn1").className="btn on";
      }
    }

    // general processing code for the web page to ask for the JSON data
    // this is actually why you need to keep sending data to the page to
    // force this call with stuff like this
    // server.on("/json", SendJSON);
    // otherwise the page will not request JSON from the ESP, and updates will not happen
    function process(){

      // only start a new request if the previous one is finished (or none has been sent yet)
      if(jsonHttp.readyState==0 || jsonHttp.readyState==4) {
        jsonHttp.open("PUT","json",true);
        jsonHttp.onreadystatechange=response;
        jsonHttp.send(null);
      }
      // you may have to play with this value, big pages need more porcessing time, and hence
      // a longer timeout
      setTimeout(process,100);
    }

  </script>

</body>

</html>

)=====";
