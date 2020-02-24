' This script contains functions for customized pane events.
' You must not modify the name of the functions.
Option Explicit

Function CustomizedPane_OnRefresh()

        LoadListView false, true

End Function





Function CustomizedPane_OnToolbarButton1Clicked()
        LoadListView true, true
End Function



Function LoadListView(IncludeComplete, FilterStockCode)

  dim ListXML
    ListXML = "<Columns PrimaryNode='Inspection' Style='DataGrid' AutoSize='true' FreezeColumn='0' >"
    ListXML = ListXML & "<Column Name='Grn' Description='Grn' />"
    ListXML = ListXML & "<Column Name='Warehouse' Description='Warehouse' />"
    ListXML = ListXML & "<Column Name='Supplier' Description='Supplier' />"
    ListXML = ListXML & "<Column Name='ReceivedDate' Description='ReceivedDate' />"
    ListXML = ListXML & "<Column Name='PurchaseOrder' Description='PurchaseOrder' />"
    ListXML = ListXML & "<Column Name='Job' Description='Job' />"
    ListXML = ListXML & "<Column Name='QtyCounted' Description='QtyCounted' />"
    ListXML = ListXML & "<Column Name='QtyInspected' Description='QtyInspected' />"
    ListXML = ListXML & "<Column Name='QtyAccepted' Description='QtyAccepted' />"
    ListXML = ListXML & "<Column Name='QtyRejected' Description='QtyRejected' />"
    ListXML = ListXML & "<Column Name='SysproOperator' Description='SysproOperator' />"
    ListXML = ListXML & "</Columns>"
    CustomizedPane.CodeObject.ListviewProperties = ListXML


	Dim conn1,rst1,sql1
	Set conn1 = CreateObject("ADODB.Connection")
	Set rst1 = CreateObject("ADODB.Recordset")
	conn1.ConnectionString = "Driver={SQL Server};Server=peerapp5;Database=PeerlessVIP;Trusted_Connection=Yes;"

	conn1.Open


    dim XmlOut
    XmlOut = "<InspectionSelect>"




        Dim conn,rst,sql
        Set conn = CreateObject("ADODB.Connection")
        Set rst = CreateObject("ADODB.Recordset")
        conn.ConnectionString = "Driver={SQL Server};Server=peerapp5;Database=" &SystemVariables.CodeObject.CompanyDatabaseName &";Trusted_Connection=Yes;"

        ' Open connection
        conn.Open

                if IncludeComplete = true AND FilterStockCode = true then
                        'Auto filter to show the last 365 days only
                sql = "SELECT i.*,pd.MJob FROM SysproCompanyP.dbo.MrpInvInspect i " _
                                  &"    INNER JOIN SysproCompanyP.dbo.PorMasterDetail pd ON i.PurchaseOrder = pd.PurchaseOrder AND i.PurchaseOrderLin = pd.Line " _
                                  &" WHERE i.StockCode='" &StockCodeDetails.CodeObject.StockCode &"' AND i.GrnReceiptDate >= DATEADD(day,-365,GETDATE()) " _
                                  &" ORDER BY i.PurchaseOrder DESC "

                else
                        if FilterStockCode = true  then
                        'Auto filter to show the last 365 days only
                sql = "SELECT i.*,pd.MJob FROM SysproCompanyP.dbo.MrpInvInspect i " _
                                  &"    INNER JOIN SysproCompanyP.dbo.PorMasterDetail pd ON i.PurchaseOrder = pd.PurchaseOrder AND i.PurchaseOrderLin = pd.Line " _
                                  &" WHERE i.StockCode='" &StockCodeDetails.CodeObject.StockCode &"' AND i.GrnReceiptDate >= DATEADD(day,-365,GETDATE()) " _
                                  &" AND i.QtyAccepted < i.QtyCounted ORDER BY i.PurchaseOrder DESC "


                        else


                        End if


                End If


        set rst = conn.execute(sql)

        Dim Operation, WorkCentre, BudgetHours, ActualHours, Variance

        while not rst.EOF

                XmlOut = XmlOut &"<Inspection>"

                XmlOut = XmlOut &"<Grn>"
                if not isnull(rst("Grn")) then XmlOut = XmlOut &cstr(rst("Grn"))
                XmlOut = XmlOut &"</Grn>"

                XmlOut = XmlOut &"<Warehouse>"
                if not isnull(rst("Warehouse")) then XmlOut = XmlOut &cstr(rst("Warehouse"))
                XmlOut = XmlOut &"</Warehouse>"

                XmlOut = XmlOut &"<Supplier>"
                if not isnull(rst("Supplier")) then XmlOut = XmlOut &cstr(rst("Supplier"))
                XmlOut = XmlOut &"</Supplier>"

                XmlOut = XmlOut &"<ReceivedDate>"
                if not isnull(rst("GrnReceiptDate")) then XmlOut = XmlOut &cstr(rst("GrnReceiptDate"))
                XmlOut = XmlOut &"</ReceivedDate>"

                XmlOut = XmlOut &"<PurchaseOrder>"
                if not isnull(rst("PurchaseOrder")) then XmlOut = XmlOut &cstr(rst("PurchaseOrder"))
                XmlOut = XmlOut &"</PurchaseOrder>"

                XmlOut = XmlOut &"<Job>"
                if not isnull(rst("MJob")) then XmlOut = XmlOut &cstr(rst("MJob"))
                XmlOut = XmlOut &"</Job>"

                XmlOut = XmlOut &"<QtyCounted>"
                if not isnull(rst("QtyCounted")) then XmlOut = XmlOut &cstr(rst("QtyCounted"))
                XmlOut = XmlOut &"</QtyCounted>"

                XmlOut = XmlOut &"<QtyInspected>"
                if not isnull(rst("QtyInspected")) then XmlOut = XmlOut &cstr(rst("QtyInspected"))
                XmlOut = XmlOut &"</QtyInspected>"

                XmlOut = XmlOut &"<QtyAccepted>"
                if not isnull(rst("QtyAccepted")) then XmlOut = XmlOut &cstr(rst("QtyAccepted"))
                XmlOut = XmlOut &"</QtyAccepted>"

                XmlOut = XmlOut &"<QtyRejected>"
                if not isnull(rst("QtyRejected")) then XmlOut = XmlOut &cstr(rst("QtyRejected"))
                XmlOut = XmlOut &"</QtyRejected>"

		XmlOut = XmlOut &"<SysproOperator>"
                if not isnull(rst("Grn")) then XmlOut = XmlOut &cstr(GetSysproOperator(cstr(rst("Grn")), conn1))
                XmlOut = XmlOut &"</SysproOperator>"

                XmlOut = XmlOut &"</Inspection>"



                rst.MoveNext
        wend



        XmlOut = XmlOut &"</InspectionSelect>"
        CustomizedPane.CodeObject.ListviewData = XmlOut

        conn.Close
		conn1.Close

End Function

Function GetSysproOperator(GrnVariable, conn1)

	dim strSQL, rst

	strSQL = "SELECT SysproOperator FROM tblPoReceiptsUser WHERE Grns LIKE '%" & GrnVariable &"%'"

	set rst = conn1.execute(strSQL)

	GetSysproOperator = ""

	while not rst.EOF
		GetSysproOperator = rst("SysproOperator")

		rst.MoveNext
	wend


End Function