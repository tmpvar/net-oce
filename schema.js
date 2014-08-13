var parse = require('protobuf-schema');
var fs = require('fs');
var path = require('path');

var obj = parse(fs.readFileSync(path.join(__dirname, 'protocol', 'oce.proto')))

var schema = module.exports = {};

obj.messages.forEach(function(message) {
  var enums = {}
  if (message.enums && message.enums.length) {
    message.enums.forEach(function(e) {
      enums[e.name] = function(str) {
        return e.values[str];
      };
    });
  }

  var fields = {};
  message.fields.forEach(function(field) {
    if (enums[field.type]) {
      fields[field.name] = enums[field.type];
    } else {
      fields[field.name] = field;
    }
  });

  schema[message.name] = fields;

});
