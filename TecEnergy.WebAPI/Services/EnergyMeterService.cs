﻿using TecEnergy.Database.Models.DataModels;
using TecEnergy.Database.Models.DtoModels;
using TecEnergy.Database.Repositories.Interfaces;
using TecEnergy.WebAPI.Helpers;
using TecEnergy.WebAPI.Mapping;

namespace TecEnergy.WebAPI.Services;

public class EnergyMeterService
{
    private readonly IEnergyMeterRepository _repository;
    public EnergyMeterService(IEnergyMeterRepository repository)
    {
        _repository = repository;
    }

    public async Task<IEnumerable<EnergyMeter>> GetAllAsync()
    {
        var result = await _repository.GetAllAsync();
        return result;
    }

    public async Task<SimpleDto> GetByIdAsync(Guid id)
    {
        var result = await _repository.GetByIdAsync(id);
        var dto = EnergyMeterMappings.EnergyMeterToSimpleDto(result);
        return dto;
    }

    //returns simpledto list of energymeters based on their roomId
    public async Task<List<SimpleDto>> GetSimpleListByRoomIdAsync(Guid id)
    {
        List<SimpleDto> dtoList = new();
        var result = await _repository.GetRoomByIdAsync(id);
        var rooms = result.EnergyMeters.ToList();
        foreach (var item in rooms)
        {
            var dto = EnergyMeterMappings.EnergyMeterToSimpleDto(item);
            dtoList.Add(dto);
        }
        return dtoList;
    }

    //return energydto for energymeter with realtime and accumulated within the timespan of datetimes
    public async Task<EnergyDto> GetByIdDatetimeAsync(Guid id, DateTime? startDateTime, DateTime? endDateTime)
    {
        var result = _repository.GetByIdDatetimeAsync(id, startDateTime, endDateTime).Result;
        var latestDatetime = result.EnergyDatas.OrderByDescending(x => x.AccumulatedValue).FirstOrDefault();
        var hoursInDouble = CalculationHelper.CalculateHoursToDouble(startDateTime, endDateTime);
        var realtime = CalculationHelper.GetKilowattsInHours(result.EnergyDatas.Count, hoursInDouble);
        var accumulated = CalculationHelper.CalculateAccumulatedEnergy(latestDatetime.AccumulatedValue, 0.001);
        var energyDto = EnergyMeterMappings.EnergyMeterToEnergyDto(result, realtime, accumulated);
        return energyDto;
    }

    public async Task CreateAsync(EnergyMeter energyMeter)
    {
        await _repository.AddAsync(energyMeter);
    }

    public async Task UpdateAsync(Guid id, EnergyMeter energyMeter)
    {
        await _repository.UpdateAsync(energyMeter);
    }

    public async Task DeleteAsync(Guid id)
    {
        await _repository.DeleteAsync(id);
    }
}
