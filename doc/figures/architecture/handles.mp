input metauml

  beginfig(1);

  %define classes

  Class.BT_HANDLE_HEADER("BT_HANDLE_HEADER")("+pIf: BT_IF_HANDLE", "+ulClaimedMemory: BT_u32", "+hNext: BT_HANDLE")();
  Class.BT_HANDLE("BT_HANDLE") ("+h: BT_HANDLE_HEADER", "-... private_data")("+BT_GetHandleInfo(BT_HANDLE, BT_HANDLE_INFO *): BT_ERROR",
	  "+BT_CloseHandle(BT_HANDLE): BT_ERROR");
  Class.Shape("Shape")() ("+get_circumference(): int");
  Class.Circle("Circle") ("-center: Point", "-radius: int")("+get_circumference(): int");

  % layout classes
  topToBottom(50)(BT_HANDLE, BT_HANDLE_HEADER);
  %leftToRight(50)(Circle, Shape);

  % draw classes
  drawObjects(BT_HANDLE, BT_HANDLE_HEADER);

  %link classes
  link(inheritance)(BT_HANDLE.s -- BT_HANDLE_HEADER.n);
  %link(composition)(BT_HANDLE.s -- BT_HANDLE_HEADER.n);


endfig;

end


