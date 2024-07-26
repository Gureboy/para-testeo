function doPost(e) {
  var hoja = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
  var usuario = e.parameter.usuario;
  var ritmoCardiaco = e.parameter.ritmoCardiaco;
  var tempCorporal = e.parameter.tempCorporal;
  hoja.appendRow([new Date(), usuario, ritmoCardiaco, tempCorporal]);
  return ContentService.createTextOutput("Datos guardados");
}
