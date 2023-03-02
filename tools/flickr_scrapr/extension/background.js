chrome.action.onClicked.addListener((tab) => {
    chrome.scripting.executeScript({
        target: { tabId: tab.id },
        files: ['content.js']
    });
});

chrome.runtime.onMessage.addListener(({ type, data }) => {
    console.log(data);
    if (type === "save-metadata") {
        fetch('http://localhost:5000/save_metadata', {
            method: 'POST', headers: {
                "Content-Type": "application/json",
            }, body: JSON.stringify(data)
        }).then(r => r.text()).then(result => {
            console.log(result);
        });
    } else if (type === "save-download") {
        fetch('http://localhost:5000/save_download', {
            method: 'POST', headers: {
                "Content-Type": "application/json",
            }, body: JSON.stringify(data)
        }).then(r => r.text()).then(result => {
            console.log(result);
        });
    }
});