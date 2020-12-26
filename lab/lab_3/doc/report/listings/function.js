var t=msg.payload.split("{")[1].split("}")[0].split(",")[2].split(":")[1].trim();
msg.payload=parseInt(t);
msg.topic="Human-firendly topic name"
return msg;