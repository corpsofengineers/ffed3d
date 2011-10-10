function OnSystemInit()

	SetLastAttackedIndex(0);
	CreateShips();

	for i=1, 0x72, 1 do

		if CheckDATAObjectArray(i, 0x4b) == false then i=i+1; end
		if i == GetPlayerIndex() then i=i+1; end

		obj = GetObject(i);

		if obj.ai_mode == AI_DOCKED_OR_LANDED then

			starport = GetObject (obj.dest_index);

			if IsStarportLocked(starport) == true then DestroyObject(obj, 0); end

		end

	end

end
