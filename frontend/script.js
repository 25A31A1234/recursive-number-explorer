async function calculate() {

    const number = document.getElementById("numberInput").value;
    const operation = document.getElementById("operationSelect").value;

    const resultArea = document.getElementById("resultArea");
    const messageArea = document.getElementById("messageArea");

    // Clear old messages
    resultArea.innerText = "";
    messageArea.innerText = "";

    // Check number
    if (number === "") {
        messageArea.innerText = "Please enter a number.";
        return;
    }

    resultArea.innerText = "Calculating...";

    // Convert frontend operation names to backend names
    const operationMap = {
        sumOfDigits: "sum",
        reverseNumber: "reverse",
        countDigits: "count",
        palindromeCheck: "palindrome",
        factorial: "factorial",
        power: "power",
        fibonacci: "fibonacci"
    };

    const backendOperation = operationMap[operation];

    try {

        const response = await fetch(
            "http://localhost:8080/calculate",
            {
                method: "POST",

                headers: {
                    "Content-Type": "application/json"
                },

                body: JSON.stringify({
                    number: Number(number),
                    operation: backendOperation
                })
            }
        );

        const data = await response.json();

        if (data.error) {
            resultArea.innerText = "";
            messageArea.innerText = data.error;
        }
        else {
            messageArea.innerText = "";
            resultArea.innerText = "Result: " + data.result;
        }

    }
    catch (error) {

        resultArea.innerText = "";

        messageArea.innerText =
            "Cannot connect to backend. Make sure main.exe is running.";

        console.error(error);
    }
}