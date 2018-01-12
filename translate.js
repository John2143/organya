const fs = require("fs");
const https = require("https");

let dryRun = process.argv.includes("--dry");

const apiKey = process.env.GOOGLE_API_KEY;
if(!apiKey) dryRun = true;

/*async*/ function POST(data){
    return new Promise((resolve, reject) => {
        let buffer = "";
        let req = https.request({
            method: "POST",
            hostname: "translation.googleapis.com",
            path: `/language/translate/v2?key=${apiKey}&source=ja`,
            headers: {
                "Content-Length": Buffer.byteLength(data),
            },
        }, res => {
            res.on("data", d => buffer += d)
            res.on("end", () => resolve(buffer));
            res.on("error", err => reject(err));
        });
        req.on("error", err => reject(err));
        req.write(data);
        req.end();
    });
}

function createPayload(array){
    let payload = "{";
    for(let string of array){
        payload += `'q': '${string}',\n`;
    }
    payload += "'target': 'en'}";
    return payload;
}

let dryRunIdx = 0;
async function getTranslation(payload){
    if(dryRun){
        return ["A", "B", "C", "D", "E"].map(x => x + dryRunIdx++);
    }else{
        try{
            let ret = await POST(payload);
            let json = JSON.parse(ret);

            return json.data.translations.map(x => x.translatedText);
        }catch(e){
            return Array(5).fill("<unknown>");
        }
    }
}

async function translate(strings){
    payloads = [];
    let tempArr = [];
    for(let obj of strings){
        tempArr.push(obj.string);
        if(tempArr.length >= 5){
            payloads.push(createPayload(tempArr));
            tempArr = [];
        }
    }
    if(tempArr.length > 0) payloads.push(createPayload(tempArr));

    let promises = [];
    for(payload of payloads){
        promises.push(getTranslation(payload));
    }
    let returnedTranslations = await Promise.all(promises);
    let translations = returnedTranslations.reduce(
        (acc, value) => {acc.push(...value); return acc},
        []
    );
    return translations;
}

let filename = process.argv[2];
console.log(`translating ${filename}`);
let file = fs.readFileSync(`source/${filename}`, {encoding: "utf-8"});

let regex = /[\u3000-\u303f\u3040-\u309f\u30a0-\u30ff\uff00-\uff9f\u4e00-\u9faf\u3400-\u4dbf]+/gi

let stringsToTranslate = [];

let matches;
while((matches = regex.exec(file)) !== null){
    let obj = {
        string: matches[0],
        index: matches.index,
        length: matches[0].length,
    };
    stringsToTranslate.push(obj);
}

translate(stringsToTranslate).then(translations => {
    let newFile = "";
    let oldFileIndex = 0;
    let translationIndex = 0;
    for(let obj of stringsToTranslate){
        newFile += file.substring(oldFileIndex, obj.index);
        newFile += translations[translationIndex++];
        oldFileIndex = obj.index + obj.length;
    }
    newFile += file.substring(oldFileIndex);

    return newFile;
}).then(data => {
    fs.writeFileSync(`source_translated/${filename}`, data, {encoding: "utf-8"});
}).catch(err => {
    console.log(err);
})
