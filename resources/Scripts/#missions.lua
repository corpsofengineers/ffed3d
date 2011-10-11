function OnSystemInit2()

	SetLastAttakedIndex(0);
	CreateShips();

	for index=1, 0x72, 1 do

		if GetObjectState(i) ~= IST_DOCKED and i ~= GetPlayerIndex() then

			obj = GetObject(i);

			if obj.ai_mode == AI_DOCKED_OR_LANDED then

				starport = GetObject(obj.dest_index);

				if IsStarportLocked(startport) == true then DestroyObject(obj, 0) end

			end

		end

	end

end
