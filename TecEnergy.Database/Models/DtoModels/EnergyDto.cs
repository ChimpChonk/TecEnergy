﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TecEnergy.Database.Models.DtoModels;
public class EnergyDto
{
    public Guid Id { get; set; }
    public string Name { get; set; }
    public double RealTime { get; set; }
    public double Accumulated { get; set; }
}
