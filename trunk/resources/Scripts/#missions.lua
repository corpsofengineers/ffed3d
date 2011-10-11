function OnSystemInit()

	SetLastAttackedIndex(0);
	CreateShips();

	for index=1, 0x72, 1 do

		if GetObjectState(index) ~= IST_DOCKED then

			obj = GetObject(index);

			if obj.ai_mode == AI_DOCKED_OR_LANDED and index ~= GetPlayerIndex() then

				starport = GetObject(obj.dest_index);

				if IsStarportLocked(starport) == true then DestroyObject(obj, 0) end

			end

		end

	end

end
