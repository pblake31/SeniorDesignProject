function publishNumber() {
  //Create a new Client object with your broker's hostname, port and your own clientId
  var client = new Paho.MQTT.Client('broker_name', 8000, 'clientId');

  var options = {
  //connection attempt timeout in seconds
  timeout: 3,
  
  onSuccess: function () {
      //select component dropdown (Topic)
      var dropdown = document.getElementById("dropdown").value;
      
      //add value from dropdown menu to end of publish topic
      var topic = 'Publish/Topic' + dropdown;
      
      //select quantity (Message)
      var quantity = document.getElementById("quantity-input").value; 
      var message = new Paho.MQTT.Message(quantity);
      
      //publish messsage
      message.destinationName = topic;
      message.qos = 2;
      client.send(message);
      
      //Alert Message that sends user back to homepage
      alert("Connection Successful: Your components will be dispensed from the Smart Shelf. Click OK to return home");
      window.location.href = "index.html";
  },

  onFailure: function (message) {
      console.log("Connection failed: " + message.errorMessage);
      alert("Connection failed: " + message.errorMessage);
  }
};

//Attempt to connect
client.connect(options);
};
