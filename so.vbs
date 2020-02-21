' This script contains functions for form and field events.
' You must not modify the name of the functions.
Option Explicit


Dim dataSource, userId, password, database, sSQLConnectionString

'Set the database variables
dataSource = "167.114.201.168" 'sql server"
userId = "sa" 'SQL User
password = "67dart" 'SQL User Password
database = "SysproOutdoors"

sSQLConnectionString = "Provider=SQLOLEDB;Data Source=" & dataSource & ";Initial Catalog=" & database & ";Persist Security Info=False;User ID=" & userId & ";Password=" & password & ";"



Function StockedLine_OnSubmit()
	
	exit function

	'Get the line values for this sales order
	Dim conn, strSql, rst
	Set conn = CreateObject("ADODB.Connection")
	
	conn.ConnectionString = sSQLConnectionString
	conn.Open

	strSql = "SELECT * FROM tCustProductClasses WHERE Customer = '" & CustomerInformation.CodeObject.Customer & "' AND ProductClass = '" & StockedLine.CodeObject.ProductClass & "'"

	Set rst = conn.Execute(strSql)

	if rst.EOF then
		msgbox("Product Class " & StockedLine.CodeObject.ProductClass & " is not allowed for customer: " & CustomerInformation.CodeObject.Customer)
		StockedLine_OnSubmit = false
		Exit Function
	end if

End Function




Function ProductClass_OnAfterChange()
	


	Dim dataSource, userId, password, database, sSQLConnectionString

	'Set the database variables
	dataSource = "167.114.201.168" 'sql server"
	userId = "sa" 'SQL User
	password = "67dart" 'SQL User Password	


	database = SystemVariables.CodeObject.CompanyDatabaseName
	sSQLConnectionString = "Provider=SQLOLEDB;Data Source=" & dataSource & ";Initial Catalog=" & database & ";Persist Security Info=False;User ID=" & userId & ";Password=" & password & ";"


	'Get the line values for this sales order
	Dim conn, strSql, rst
	Set conn = CreateObject("ADODB.Connection")
	
	conn.ConnectionString = sSQLConnectionString
	conn.Open

	strSql = "SELECT * FROM tCustProductClasses WHERE Customer = '" & CustomerInformation.CodeObject.Customer & "' AND ProductClass = '" & StockedLine.CodeObject.ProductClass & "'"

	Set rst = conn.Execute(strSql)

	if rst.EOF then
		msgbox("Product Class " & StockedLine.CodeObject.ProductClass & " is not allowed for customer " & CustomerInformation.CodeObject.Customer)

		strSql = "INSERT INTO tApplicationLog (LogDate, Application, Doc, Line, SKU, Operator, MessageType, Message) "
	    strSql = strSql & "VALUES ('" & Now & "', 'Restricted Product Classes', '" & OrderHeader.CodeObject.SalesOrder & "'"
	    strSql = strSql & ", '', '" & StockedLine.CodeObject.StockCode & "', '" & SystemVariables.CodeObject.Operator & "'"
	    strSql = strSql & ", 'Error', 'User attempted to add restricted product class to sales order. Operator: " & SystemVariables.CodeObject.Operator & ", Stock Code: " & StockedLine.CodeObject.StockCode & ", Product Class: " & StockedLine.CodeObject.ProductClass & ", Customer: " & CustomerInformation.CodeObject.Customer & ".'); "

		conn.Execute(strSql)

		Exit Function
	end if



End Function


Function StockedLine_OnRefresh()
	
	msgbox("on refresh")
	msgbox(StockedLine.CodeObject.ProductClass)
	if(StockedLine.CodeObject.ProductClass = "") then
		exit function
	end if

End Function


' This script contains functions for form and field events.
' You must not modify the name of the functions.
Option Explicit


Dim dataSource, userId, password, database, sSQLConnectionString

'Set the database variables
dataSource = "167.114.201.168" 'sql server"
userId = "sa" 'SQL User
password = "67dart" 'SQL User Password
database = "SysproOutdoors"

sSQLConnectionString = "Provider=SQLOLEDB;Data Source=" & dataSource & ";Initial Catalog=" & database & ";Persist Security Info=False;User ID=" & userId & ";Password=" & password & ";"


Function EnteredOrderLines_OnPopulate()

Dim i, currentStockCode, count

count = 0

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'Iterate the entered stock lines grid
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
for i = 0 to UBOUND(EnteredOrderLines.CodeObject.Array,2)-1

	currentStockCode = EnteredOrderLines.CodeObject.Array(001,i)

	if(StockedLine.CodeObject.StockCode = currentStockCode) then
		count = count + 1
	end if

Next

	if count > 1 then
		msgbox("Error! Stock code "& currentStockCode &" entered in sales order twice.")
	elseif count = 1 then
		msgbox("Please note that currently selected stock code "& currentStockCode &" has been entered in sales order.")
	end if		

End Function