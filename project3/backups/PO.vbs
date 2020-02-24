' This script contains functions for list view events.
' You must not modify the name of the functions.
Option Explicit
Dim strConnectionStringPeerlessVIP, conn, strSQL


Function WhereusedQuery_OnPopulate()


Dim i, StockCode, strReturn



	If isobject(conn) = false Then
				
		strConnectionStringPeerlessVIP = "Driver={SQL Server};Server=peerapp5;Database=PeerlessVIP;Trusted_Connection=Yes;"  
		Set conn = CreateObject("ADODB.Connection")
		conn.ConnectionString = strConnectionStringPeerlessVIP
		
		' Open connection
		conn.Open
				
	End If
	

StockCode = ""
strReturn = ""

	

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'Iterate the grid
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
for i = 0 to UBOUND(WhereusedQuery.CodeObject.Array,2)-1


    'Update the "Rev" column

    StockCode = WhereusedQuery.CodeObject.Array(000,i)
    strReturn = cstr(GetLongDesc(StockCode))

	strReturn = Replace(strReturn, chr(38), "&amp;") ' AMPERSAND
	strReturn = Replace(strReturn, chr(34), "&quot;") ' QUOTES
	strReturn = Replace(strReturn, chr(39), "&apos;")  ' APOSTROPHES
	strReturn = Replace(strReturn, chr(60), "&lt;") ' LESS THAN
	strReturn = Replace(strReturn, chr(62), "&gt;") ' GREATER THAN

	WhereusedQuery_OUT.CodeObject.Array(007,i) = "<Field Value='" & strReturn & "'> </Field>"




Next


conn.close

End Function




Function GetLongDesc(StockCode)
	
	Dim rst, result, strSQL,iRowLoop
	Set rst = CreateObject("ADODB.Recordset")	


	strSQL = "SELECT LongDesc " _
			&"FROM SysproCompanyP.dbo.InvMaster WHERE StockCode = '"& StockCode &"' "
	
	set rst = conn.execute(strSQL)



	if isNull(result) then
		GetLongDesc = ""
	else

		GetLongDesc = Trim(cstr(rst("LongDesc")))
	end if



end function

