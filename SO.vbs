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




' Function StockedLine_OnSubmit()
	
' 	exit function

' 	'Get the line values for this sales order
' 	Dim conn, strSql, rst
' 	Set conn = CreateObject("ADODB.Connection")
	
' 	conn.ConnectionString = sSQLConnectionString
' 	conn.Open

' 	strSql = "SELECT * FROM tCustProductClasses WHERE Customer = '" & CustomerInformation.CodeObject.Customer & "' AND ProductClass = '" & StockedLine.CodeObject.ProductClass & "'"

' 	Set rst = conn.Execute(strSql)

' 	if rst.EOF then
' 		msgbox("Product Class " & StockedLine.CodeObject.ProductClass & " is not allowed for customer: " & CustomerInformation.CodeObject.Customer)
' 		StockedLine_OnSubmit = false
' 		Exit Function
' 	end if

' End Function




' Function ProductClass_OnAfterChange()
	


' 	Dim dataSource, userId, password, database, sSQLConnectionString

' 	'Set the database variables
' 	dataSource = "167.114.201.168" 'sql server"
' 	userId = "sa" 'SQL User
' 	password = "67dart" 'SQL User Password	


' 	database = SystemVariables.CodeObject.CompanyDatabaseName
' 	sSQLConnectionString = "Provider=SQLOLEDB;Data Source=" & dataSource & ";Initial Catalog=" & database & ";Persist Security Info=False;User ID=" & userId & ";Password=" & password & ";"


' 	'Get the line values for this sales order
' 	Dim conn, strSql, rst
' 	Set conn = CreateObject("ADODB.Connection")
	
' 	conn.ConnectionString = sSQLConnectionString
' 	conn.Open

' 	strSql = "SELECT * FROM tCustProductClasses WHERE Customer = '" & CustomerInformation.CodeObject.Customer & "' AND ProductClass = '" & StockedLine.CodeObject.ProductClass & "'"

' 	Set rst = conn.Execute(strSql)

' 	if rst.EOF then
' 		msgbox("Product Class " & StockedLine.CodeObject.ProductClass & " is not allowed for customer " & CustomerInformation.CodeObject.Customer)

' 		strSql = "INSERT INTO tApplicationLog (LogDate, Application, Doc, Line, SKU, Operator, MessageType, Message) "
' 	    strSql = strSql & "VALUES ('" & Now & "', 'Restricted Product Classes', '" & OrderHeader.CodeObject.SalesOrder & "'"
' 	    strSql = strSql & ", '', '" & StockedLine.CodeObject.StockCode & "', '" & SystemVariables.CodeObject.Operator & "'"
' 	    strSql = strSql & ", 'Error', 'User attempted to add restricted product class to sales order. Operator: " & SystemVariables.CodeObject.Operator & ", Stock Code: " & StockedLine.CodeObject.StockCode & ", Product Class: " & StockedLine.CodeObject.ProductClass & ", Customer: " & CustomerInformation.CodeObject.Customer & ".'); "

' 		conn.Execute(strSql)

' 		Exit Function
' 	end if



' End Function


' Function StockedLine_OnRefresh()
	
' 	msgbox("on refresh")
' 	msgbox(StockedLine.CodeObject.ProductClass)
' 	if(StockedLine.CodeObject.ProductClass = "") then
' 		exit function
' 	end if

' End Function



Function StockCode_OnLostFocus()

	Dim stockCodes, stockCode

	stockCodes = SystemVariables.CodeObject.GlobalVariable1
	
	' make sure string is not empty in case it is the first order line being entered
	if (Len(stockCodes) > 0) then
		stockCodes = LEFT(stockCodes, Len(stockCodes)-1) 'remove last comma on the right
	
		stockCodes = Split(stockCodes,",")
	
		For each stockCode in stockCodes
			' duplicate found in Ordered Lines Grid
			if (stockCode = StockedLine.CodeObject.StockCode) then
				msgbox("Error! Stock code already entered in current sales order")
			end if
	
		Next

	end if



End Function
