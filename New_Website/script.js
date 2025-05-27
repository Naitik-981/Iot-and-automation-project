document.addEventListener("DOMContentLoaded", function () {
    const channelID = '2717430';
    const ledchannelID = '2753822';
    const writeAPIKey = '5KSKJ52T5SAV89DU';
    let value = 0;

    async function writeData() {
        try {


            // Step 3: Make bulk write API call
            const writeResponse = await fetch(`https://api.thingspeak.com/update?api_key=7MB9CVJ8JW34ORGP&field1=${value}`);
            const result = await writeResponse.json();
            console.log("Write Response:", result);

            // Step 4: Verify and update UI
            if (result == 0) {
                console.error("Write Unsuccessful:", result.error.message);
            } else {
                console.log("Write Successful");
                value = 1 - value; // Toggle the LED value
                document.getElementById("led-status").textContent = value === 0 ? "ON" : "OFF";
                document.getElementById("led-status").className = value === 0 ? "on" : "off";
            }
        } catch (error) {
            console.error("An error occurred during write:", error);
        }
    }

    async function fetchData() {
        try {
            // Fetch the latest data from ThingSpeak API
            const response = await fetch(`https://api.thingspeak.com/channels/${channelID}/feeds/last.json`);
            const ans = await response.json();
            console.log("Raw Response:", ans);

            // Parse and assign values
            const humidity1 = ans.field1 !== null ? parseFloat(ans.field1) : "No Data";
            const temp1 = ans.field2 !== null ? parseFloat(ans.field2).toFixed(2) : "No Data";
            const humidity2 = ans.field3 !== null ? parseFloat(ans.field3) : "No Data";
            const temp2 = ans.field4 !== null ? parseFloat(ans.field4).toFixed(2) : "No Data";

            // Update the HTML elements
            document.getElementById("humidity1").textContent = humidity1;
            document.getElementById("temperature1").textContent = temp1;
            document.getElementById("humidity2").textContent = humidity2;
            document.getElementById("temperature2").textContent = temp2;

            console.log("Data updated successfully!");

            const ledresponse = await fetch(`https://api.thingspeak.com/channels/2753822/fields/1/last.json`);
            const ledans = await ledresponse.json();
            console.log("LED STATUS: ",ledans.field1);
            document.getElementById("led-status").textContent = ledans.field1 == 1 ? "ON" : "OFF";
            document.getElementById("led-status").className = ledans.field1 == 1 ? "on" : "off";

        } catch (error) {
            console.error("Error fetching or parsing data:", error);
        }
    }

    // Fetch data when the page loads
    fetchData();
    // Poll data every 30 seconds
    setInterval(fetchData, 15000);

    document.getElementById("led").addEventListener("click", () => {
        console.log("Button Clicked");
        writeData();
    });
});
