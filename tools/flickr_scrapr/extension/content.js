function scrape_metadata() {
    var attribution = document.querySelector('a.owner-name');
    var attribution_name = attribution.innerHTML.trim();
    var attribution_url = 'https://www.flickr.com' + attribution.getAttribute('href');

    var license_url = document.querySelector('a.photo-license-url').getAttribute('href');
    var license_text = document.querySelector('a.photo-license-url span').innerHTML.trim()
    var page_url = document.head.querySelector("[property~='og:url'][content]").content;
    var download_link = document.querySelector('div.download a');
    if (download_link) {
        var download_page_url = 'https://www.flickr.com' + download_link.getAttribute('href');

        var page_metadata = { attribution_name, attribution_url, license_url, license_text, page_url, download_page_url };
        chrome.runtime.sendMessage({ type: "save-metadata", data: page_metadata });
    }
}

function scrape_download() {
    var download_link = document.evaluate("//a[contains(.,'Download the')]", document, null, XPathResult.ANY_TYPE, null).iterateNext();
    var photo_backlink = document.evaluate("//h1/a[contains(.,'Photo')]", document, null, XPathResult.ANY_TYPE, null).iterateNext();

    chrome.runtime.sendMessage(
        {
            type: "save-download",
            data: {
                metadata_url: 'https://www.flickr.com' + photo_backlink.getAttribute('href'),
                download_page_url: document.URL,
                download_url: download_link.getAttribute('href')
            }
        });
}

function scrape() {
    var download_link = document.evaluate("//a[contains(.,'Download the')]", document, null, XPathResult.ANY_TYPE, null).iterateNext();
    if (download_link) {
        scrape_download();
    } else {
        scrape_metadata();
    }
}

if (document.URL.startsWith("https://www.flickr.com/photos/")) {
    scrape();
}