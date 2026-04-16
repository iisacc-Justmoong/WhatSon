pragma ComponentBehavior: Bound

import QtQuick

QtObject {
    id: parser

    function appendResourceDropPayloadLines(rawText, urls) {
        if (!Array.isArray(urls))
            return;
        const normalizedText = rawText === undefined || rawText === null ? "" : String(rawText).trim();
        if (normalizedText.length === 0)
            return;
        const payloadLines = normalizedText.split(/\r?\n|\u0000/g);
        for (let lineIndex = 0; lineIndex < payloadLines.length; ++lineIndex) {
            const line = String(payloadLines[lineIndex] || "").trim();
            if (line.length === 0 || line.charAt(0) === "#")
                continue;
            urls.push(line);
        }
    }

    function appendResourceDropMimePayload(drop, mimeType, urls) {
        if (!drop
                || drop.getDataAsString === undefined
                || !Array.isArray(urls)) {
            return;
        }
        parser.appendResourceDropPayloadLines(drop.getDataAsString(mimeType), urls);
    }

    function extractResourceDropUrls(drop) {
        const urls = [];
        if (drop && drop.urls !== undefined && drop.urls !== null) {
            if (Array.isArray(drop.urls)) {
                for (let index = 0; index < drop.urls.length; ++index)
                    urls.push(drop.urls[index]);
            } else if (drop.urls.length !== undefined) {
                for (let listIndex = 0; listIndex < drop.urls.length; ++listIndex)
                    urls.push(drop.urls[listIndex]);
            } else {
                urls.push(drop.urls);
            }
        }
        if (urls.length > 0)
            return urls;

        parser.appendResourceDropPayloadLines(drop && drop.text !== undefined ? drop.text : "", urls);
        const mimeTypes = [
            "text/uri-list",
            "text/plain",
            "public.file-url",
            "public.url",
            "text/x-moz-url"
        ];
        for (let index = 0; index < mimeTypes.length; ++index)
            parser.appendResourceDropMimePayload(drop, mimeTypes[index], urls);
        return urls;
    }
}
