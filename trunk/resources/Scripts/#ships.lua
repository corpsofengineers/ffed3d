function CreateShips_callback_()

	for i=0, TransportMissions(), 1 do

		mission = GetTrsnsportMission(i);

		if mission.passengers == 0 and mission.mission_idx >= 0x20 then

			if logic_and(mission.mission_idx, 0x40) > 0 then
				mission.mission_idx = 0x21;
				mission.name = 0xffffffff;
			else
				mission.mission_idx = 0x20;
				mission.name = 0xfffffffe;
			end

		else

			 mission.mission_idx = logic_shiftright (mission.mission_idx, 1);

		end

		if mission.system == CurrentSystem() then
			SpawnAssassins(mission.ships, mission.mission_idx, mission.name);
		else
			SpawnAssassins(sqrt(mission.ships), mission.mission_idx, mission.name);
		end


	end

	SpawnPirates(CurrentPirates(), 1);
	SpawnTraders(CurrentTraders(), 1);

end
