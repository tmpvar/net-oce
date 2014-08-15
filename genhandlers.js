var fs = require('fs');
var path = require('path');
var result = fs.readFileSync(
  path.join(__dirname, 'handler.h.in'),
  'utf8'
);

var outPath = path.join(__dirname, 'out');

if (!fs.existsSync(outPath)) {
  fs.mkdirSync(outPath);
}

var files = fs.readdirSync(path.join(__dirname, 'protocol', 'handlers'));

function r(name, value) {
  result = result.replace('/* ' + name + ' */', value);
}

var headers = files.map(function(header) {
  return '  #include "protocol/handlers/' + header + '"';
});

r('HEADERS', headers.join('\n'));
r('HANDLER_COUNT', headers.length + 1);

var handlers = files.map(function(header) {
  return header.replace('.h', '');
});

var inspectResponse = handlers.map(function(handler, i) {
  return [
    '',
    '    // ' + handler,
    '    value = res->add_value();',
    '    value->set_type(NetOCE_Value::OPERATION);',
    '    operation = new NetOCE_Operation();',
    '    operation->set_id(' + (i+1) +');',
    '    operation->set_name("' + handler + '");',
    '    operation->set_arguments(net_oce_argument_hint_' + handler + ');',
    '    value->set_allocated_operation(operation);',
  ].join('\n')
});

r('INSPECT_RESPONSE', inspectResponse.join('\n'));

var handlerMap = files.map(function(header) {
  return '    HANDLER_FROM_BASIC(' + header.replace('.h', '') + ')'
});

r('HANDLER_LINES', handlerMap.join(',\n'));

if (process.argv[2]) {
  fs.writeFileSync(process.argv[2], result);
} else {
  console.log(result);
}
