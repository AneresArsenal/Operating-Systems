' This script contains functions for list view events.
' You must not modify the name of the functions.
Option Explicit

dim USPrice, USExchRate, USCurrency
dim a_ReceiptData, a_POData, a_TaxData, a_HistoryData
Dim Status

Dim strConnectionStringPeerlessVIP, conn, strSQL


Function LoadReceiptData
	
	Dim rst
	Set rst = CreateObject("ADODB.Recordset")	

	strSQL = "SELECT PurchaseOrderLin, SUM(QtyAdvised) as QtyAdvised, SUM(QtyAccepted) as QtyAccepted, SUM(QtyRejected) as QtyRejected " _
			&"FROM SysproCompanyP.dbo.InvInspect WHERE PurchaseOrder = '" &OrderHeader.CodeObject.PurchaseOrder &"' GROUP BY PurchaseOrderLin "
	
	set rst = conn.execute(strSQL)

	'Load the array
	if rst.BOF AND rst.EOF then
		a_ReceiptData = null
	else
		a_ReceiptData = rst.GetRows()
	end if


end function



Function GetReceiptData(POLine)

	Dim iRowLoop, iColLoop, dblQtyAdvised,dblQtyAccepted, dblQtyRejected, strFieldReturn 
	
	dblQtyAdvised = 0
	dblQtyAccepted = 0
	dblQtyRejected = 0
	
	if IsNull(a_ReceiptData) then 
		strFieldReturn = ""
	
	else
		
		For iRowLoop = 0 to UBound(a_ReceiptData, 2)
			if cstr(a_ReceiptData(0, iRowLoop)) = cstr(POLine) then
				dblQtyAdvised = dblQtyAdvised +cdbl(a_ReceiptData(1, iRowLoop))
				dblQtyAccepted = dblQtyAccepted +cdbl(a_ReceiptData(2, iRowLoop))
				dblQtyRejected = dblQtyRejected +cdbl(a_ReceiptData(3, iRowLoop))  
			end if
		Next 'iRowLoop
		
		
		'Configure the return string
		strFieldReturn = "<Field "
		
		if dblQtyRejected > 0 then
			strFieldReturn = strFieldReturn &"Background='255137100' Value='Rejected:" &cstr(dblQtyRejected) &" Accepted:"  &cstr(dblQtyAccepted) &"' "
			Status = "Received / Rejected Item(s)"
		else 
	'		if dblQtyAccepted = 0 AND dblQtyAdvised>0 then
			if dblQtyAccepted < dblQtyAdvised then
				strFieldReturn = strFieldReturn &"Background='255255000' Value='In Inspection: " &cstr(dblQtyAdvised - dblQtyAccepted) &"' " 
				Status = "Received / In Inspection"
			else
				if dblQtyAdvised = dblQtyAccepted then
					if dblQtyAccepted > 0 then
						strFieldReturn = strFieldReturn &"Background='051153102' Value='Received: " &cstr(dblQtyAccepted) &"' " 
					end if
				end if
			end if
		end if
		strFieldReturn = strFieldReturn &"> </Field>"
	
	end if
	
	GetReceiptData = strFieldReturn

end function


Function DetailLinesForPurchaseOrder_OnPopulate()

	if isnull(SystemVariables.CodeObject.GlobalVariable1) or OrderHeader.CodeObject.PurchaseOrder <> SystemVariables.CodeObject.GlobalVariable1 then

		'if SystemVariables.CodeObject.Operator = "jkrieg" then
		if SystemVariables.CodeObject.Company = "P" then
		
			Dim strStockCode, rstStockCode, sqlStockCode, result, Price_, Qty_, DueDate_,i, Taxable, history_
	
			' Check connection
			If isobject(conn) = false Then
				
				strConnectionStringPeerlessVIP = "Driver={SQL Server};Server=peerapp5;Database=PeerlessVIP;Trusted_Connection=Yes;"  
				Set conn = CreateObject("ADODB.Connection")
				conn.ConnectionString = strConnectionStringPeerlessVIP
				
				' Open connection
				conn.Open
				
			End If
	
			
			'Set the currency variable.
			SetCurrency
		
			'Load the PO Data
			if USCurrency = true then
				LoadPOData
			End if
	
			'Load the Inspection Receipt array
			LoadReceiptData
	
			'Load the tax data
			LoadTaxData
		
			'Set the header receipt status
			Status = ""
			
			Set rstStockCode = CreateObject("ADODB.Recordset")	
		
			sqlStockCode = "EXEC PeerlessVIP.dbo.sp_POLineHistorySELECT '" & OrderHeader.CodeObject.PurchaseOrder & "' "
				
			history_ = "N"
		
			If SystemVariables.CodeObject.Operator = "gdubor" _
				OR SystemVariables.CodeObject.Operator = "klberg" _
				OR SystemVariables.CodeObject.Operator = "jshuka" _
				OR SystemVariables.CodeObject.Operator = "jason2" _
				OR SystemVariables.CodeObject.Operator = "mguil" _
				OR SystemVariables.CodeObject.Operator = "rmath2" _
				OR SystemVariables.CodeObject.Operator = "rsaun1" _
				OR SystemVariables.CodeObject.Operator = "sblack" _
				OR SystemVariables.CodeObject.Operator = "jkrieg" _
				OR SystemVariables.CodeObject.Operator = "dbird" _
				then
				set rstStockCode = conn.execute(sqlStockCode)
				If not rstStockCode.eof then
					history_ = "Y"
				End If
			End If
		
			if SystemVariables.CodeObject.Operator = "dougb" then
			'	msgbox GetReceiptData("26")
			end if
	
			For i = 0 to UBound(DetailLinesForPurchaseOrder.CodeObject.Array,2) - 1
		
				if DetailLinesForPurchaseOrder.CodeObject.Array(000,i) <> "" then
		
					'Populate the route field with the inspection data
					Dim strReturn
					strReturn = GetReceiptData(DetailLinesForPurchaseOrder.CodeObject.Array(012,i))
		
					'If nothing is returned then it is prior to using the instpection system.
					if trim(strReturn) = "" then
							if trim(cstr(DetailLinesForPurchaseOrder.CodeObject.Array(002,i))) = "0" then
								strReturn = "<Field Background='051153102' Value='Received before insp sys' ></Field>"
							end if
					end if
					DetailLinesForPurchaseOrder_OUT.CodeObject.Array(042,i) = strReturn
		
					'This needs to be re-written using an array.
					If history_ = "Y" then
				        rstStockCode.movefirst
						While not rstStockCode.eof
							if CDbl(DetailLinesForPurchaseOrder.CodeObject.Array(012,i)) = CDbl(rstStockCode("Line")) then
			
								if trim(rstStockCode("PriceChanged")) = "Y" then
									DetailLinesForPurchaseOrder_OUT.CodeObject.Array(007,i) = "<Field IsUnderline='true'  > </Field>"
								end if
						
								if trim(rstStockCode("QtyChanged")) = "Y" then
									DetailLinesForPurchaseOrder_OUT.CodeObject.Array(003,i) = "<Field IsUnderline='true'  > </Field>" 
								end if
						
								if trim(rstStockCode("DueDateChanged")) = "Y" then
									DetailLinesForPurchaseOrder_OUT.CodeObject.Array(009,i) = "<Field IsUnderline='true'  > </Field>"
								end if
			
								if trim(rstStockCode("CommentsExists")) = "Y" then
									DetailLinesForPurchaseOrder_OUT.CodeObject.Array(000,i) = "<Field Icon='096' > </Field>"
								end if
								
							end if
				            rstStockCode.movenext
						Wend
				
					End if
		
				end if
	
				'Check tax status
				if CheckTaxable2(DetailLinesForPurchaseOrder.CodeObject.Array(012,i)) = true then
		
					DetailLinesForPurchaseOrder_OUT.CodeObject.Array(000,i) = "<Field Background='Lightcyan'  > </Field>"
					DetailLinesForPurchaseOrder_OUT.CodeObject.Array(001,i) = "<Field Background='Lightcyan'  > </Field>"
					DetailLinesForPurchaseOrder_OUT.CodeObject.Array(002,i) = "<Field Background='Lightcyan'  > </Field>"
					DetailLinesForPurchaseOrder_OUT.CodeObject.Array(003,i) = "<Field Background='Lightcyan'  > </Field>"
					DetailLinesForPurchaseOrder_OUT.CodeObject.Array(004,i) = "<Field Background='Lightcyan'  > </Field>"
					DetailLinesForPurchaseOrder_OUT.CodeObject.Array(005,i) = "<Field Background='Lightcyan'  > </Field>"
					DetailLinesForPurchaseOrder_OUT.CodeObject.Array(007,i) = "<Field Background='Lightcyan'  > </Field>"
					DetailLinesForPurchaseOrder_OUT.CodeObject.Array(008,i) = "<Field Background='Lightcyan'  > </Field>"
					DetailLinesForPurchaseOrder_OUT.CodeObject.Array(009,i) = "<Field Background='Lightcyan'  > </Field>"
					DetailLinesForPurchaseOrder_OUT.CodeObject.Array(029,i) = "<Field Background='Lightcyan'  > </Field>"
					DetailLinesForPurchaseOrder_OUT.CodeObject.Array(030,i) = "<Field Background='Lightcyan'  > </Field>"
		
				end if
		
				'Show the currency
				if USCurrency = true then
					if DetailLinesForPurchaseOrder.CodeObject.Array(003,i) <> DetailLinesForPurchaseOrder.CodeObject.Array(002,i) then
		
						GetUSPrice2(DetailLinesForPurchaseOrder.CodeObject.Array(012,i))
						DetailLinesForPurchaseOrder_OUT.CodeObject.Array(013,i) = "<Field Value='US$ " & USPrice & "'  > </Field>"
						DetailLinesForPurchaseOrder_OUT.CodeObject.Array(014,i) = "<Field Value='Exch: " & USExchRate & "'  > </Field>"
					end if
				end if
		
			Next
	
			'Update the status if there are any rejcts or anything in inspection
			if Status <> "" then
				OrderHeader.CodeObject.Open_Arrived_Received = "<Field Background='255255153' Value='" &Status &"'  > </Field>"		
			end if
		
			If history_ = "Y" then
				rstStockCode.close
			end if
		
			conn.close
		
		end if

		SystemVariables.CodeObject.GlobalVariable1 = OrderHeader.CodeObject.PurchaseOrder

	end if

End Function


'	sqlStockCode = "EXEC PeerlessVIP.dbo.sp_POLineHistorySELECT '" & OrderHeader.CodeObject.PurchaseOrder & "' "


Function LoadHistoryData

	Dim rst
	Set rst = CreateObject("ADODB.Recordset")	

	strSQL = "SELECT SUBSTRING(KeyField, 7, 4) AS Line, AlphaValue As Taxable FROM SysproCompanyP.dbo.AdmFormData " _
			&"WHERE FieldName = 'PSTTAX' AND FormType = 'PORLIN' " _
			&"AND SUBSTRING(KeyField, 1, 6) = '" & OrderHeader.CodeObject.PurchaseOrder & "' "

	set rst = conn.execute(strSQL)

	'Load the array
	if rst.BOF AND rst.EOF then
		a_HistoryData = null
	else
		a_HistoryData = rst.GetRows()
	end if

end function




Function LoadTaxData

	Dim rst
	Set rst = CreateObject("ADODB.Recordset")	
	
	strSQL = "SELECT SUBSTRING(KeyField, 7, 4) AS Line, AlphaValue As Taxable FROM SysproCompanyP.dbo.AdmFormData " _
			&"WHERE FieldName = 'PSTTAX' AND FormType = 'PORLIN' " _
			&"AND SUBSTRING(KeyField, 1, 6) = '" & OrderHeader.CodeObject.PurchaseOrder & "' "
	
	set rst = conn.execute(strSQL)

	'Load the array
	if rst.BOF AND rst.EOF then
		a_TaxData = null
	else
		a_TaxData = rst.GetRows()
	end if

end function



Function CheckTaxable2(line) 

	Dim iRowLoop, iColLoop, dblQtyAdvised,dblQtyAccepted, dblQtyRejected, result
	
	result = false 
	
	if IsNull(a_TaxData) then 
	else
		
		For iRowLoop = 0 to UBound(a_TaxData, 2)
			if cstr(a_TaxData(0, iRowLoop)) = cstr(line) then
	
				if trim(a_TaxData(1, iRowLoop)) = "Y" then
					result = true 
				end if
	
				exit for
			end if
		Next 'iRowLoop
		
	End if
	
	CheckTaxable2 = result

end Function


Function CheckTaxable(line)

	Dim strStockCode, rstStockCode, sqlStockCode, result
	Set rstStockCode = CreateObject("ADODB.Recordset")	

	if SystemVariables.CodeObject.Company = "T" then
		sqlStockCode = "EXEC PeerlessVIP.dbo.spSysproNonStockedCodePstTaxSelect_T '" & OrderHeader.CodeObject.PurchaseOrder & "', '" & trim(line) & "' "
	End If

	if SystemVariables.CodeObject.Company = "P" then
		sqlStockCode = "EXEC PeerlessVIP.dbo.spSysproNonStockedCodePstTaxSelect '" & OrderHeader.CodeObject.PurchaseOrder & "', '" & trim(line) & "' "
	End If

		
	set rstStockCode = conn.execute(sqlStockCode)
		
	If not rstStockCode.eof then
		if trim(rstStockCode("Taxable")) = "Y" then
			result = true 'rstStockCode("Taxable") 
		else
			result = false 
		end if
	else
		result = false
	End if

	CheckTaxable = result

End Function



Sub SetCurrency

	''''''''''''''''''''''''''''''''''''''
	'Set the USCurrency variable.
	''''''''''''''''''''''''''''''''''''''
	Dim rst
	Set rst = CreateObject("ADODB.Recordset")	

	strsql = "SELECT Currency FROM SysproCompanyP.dbo.ApSupplier WHERE Supplier = '" & OrderHeader.CodeObject.Supplier & "' AND Currency = 'USD' "
	set rst = conn.execute(strsql)		

	if rst.BOF AND rst.EOF then
		USCurrency = true 
	else
		USCurrency = true 
	end if

end sub



Function LoadPOData

	Dim rst
	Set rst = CreateObject("ADODB.Recordset")	

	strSQL = "SELECT PurchaseOrderLin, PriceReceived, ExchangeRate FROM SysproCompanyP.dbo.PorHistReceipt WHERE PurchaseOrder = '" & OrderHeader.CodeObject.PurchaseOrder & "' "
	
	set rst = conn.execute(strSQL)

	'Load the array
	if rst.BOF AND rst.EOF then
		a_POData = null
	else
		a_POData = rst.GetRows()
	end if

end function


Sub GetUSPrice2(line)  

	Dim iRowLoop, iColLoop, dblQtyAdvised,dblQtyAccepted, dblQtyRejected, strFieldReturn 
	USPrice = ""
	USExchRate = ""
	
	if not IsNull(a_POData) then 
		For iRowLoop = 0 to UBound(a_POData, 2)
			if cstr(a_POData(0, iRowLoop)) = cstr(line) then
	
				USPrice = a_POData(1, iRowLoop)
				USExchRate = a_POData(2, iRowLoop)
	
				exit for
			end if
		Next 'iRowLoop
	End if

end sub




Function CheckChanges(line)

'this function is not reqired more 

	Dim strStockCode, rstStockCode, sqlStockCode, result, Price_, Qty_, DueDate_
	Set rstStockCode = CreateObject("ADODB.Recordset")	

	if SystemVariables.CodeObject.Company = "P" then
		sqlStockCode = "EXEC PeerlessVIP.dbo.sp_POLineHistorySELECT '" & OrderHeader.CodeObject.PurchaseOrder & "', '" & trim(line) & "' "
	End If

	set rstStockCode = conn.execute(sqlStockCode)
		
	If not rstStockCode.eof then
		if trim(rstStockCode("PriceChanged")) = "Y" then
			Price_ = "Y" 
		else
			Price_ = "N" 
		end if

		if trim(rstStockCode("QtyChanged")) = "Y" then
			Qty_ = "Y" 
		else
			Qty_ = "N" 
		end if

		if trim(rstStockCode("DueDateChanged")) = "Y" then
			DueDate_ = "Y" 
		else
			DueDate_ = "N" 
		end if

		result = Price_ & Qty_ & DueDate_

	else
		result = "NNN"
	End if

	rstStockCode.close

	CheckChanges = result

End Function





Function DetailLines_OnPopulate()

End Function