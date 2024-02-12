
using Microsoft.EntityFrameworkCore;
using WebApi.Data;

namespace WebApi
{
    public class Program
    {
        public static void Main(string[] args)
        {
            var builder = WebApplication.CreateBuilder(args);

            // Add services to the container.

            ////Energy Awareness Room connectionstring
            //builder.Services.AddDbContext<DatabaseContext>(
            //         o => o.UseSqlServer("Data Source=10.233.134.112,1433;Initial Catalog=TecEnergyDB;User ID=TecAdmin;Password=Tec420;TrustServerCertificate=True"));

            //SKP Room connectionstring
            builder.Services.AddDbContext<DatabaseContext>(
                     o => o.UseSqlServer("Data Source=192.168.21.7,1433;Initial Catalog=EnergyMonitor7;User ID=Admin;Password=Tec420;TrustServerCertificate=True"));


            // configure user secrets:
            // 1. use: " dotnet user-secrets init " to initialize secrets for the project (remember to cd into the correct project)
            // 2. use: " dotnet user-secrets set "ConnectionString" "actual-connectionstring" " to set the connectionstring ( don't include password here)
            // 3. use: " dotnet user-secrets set "DbPass" "password" "
            // to view the secrets.json right click the project (TecEnergy.WebAPI) and chose manage user 
            // documentation: https://learn.microsoft.com/en-us/aspnet/core/security/app-secrets?view=aspnetcore-8.0&tabs=windows

            //var conStrBuilder = new SqlConnectionStringBuilder();
            //conStrBuilder.ConnectionString = builder.Configuration["ConnectionString"];
            //conStrBuilder.Password = builder.Configuration["DbPass"];
            //var conn = conStrBuilder.ConnectionString;

            //builder.Services.AddDbContext<DatabaseContext>(
            //    o => o.UseSqlServer(conn));

            builder.Services.AddCors(options =>
            {
                var allowedOrigins = "*";

                options.AddDefaultPolicy(policy =>
                {

                    policy.WithOrigins(allowedOrigins)
                          .WithHeaders("Content-Type", "Authorization", "Access-Control-Allow-Headers", "Access-Control-Allow-Origin")
                          .WithMethods("GET", "POST", "PUT", "DELETE", "PATCH");
                });
            });



            builder.Services.AddControllers();
            // Learn more about configuring Swagger/OpenAPI at https://aka.ms/aspnetcore/swashbuckle
            builder.Services.AddEndpointsApiExplorer();
            builder.Services.AddSwaggerGen();

            var app = builder.Build();

            // Configure the HTTP request pipeline.
            if (app.Environment.IsDevelopment())
            {
                app.UseSwagger();
                app.UseSwaggerUI();
            }

            app.UseHttpsRedirection();

            app.UseCors();

            app.UseAuthorization();


            app.MapControllers();

            app.Run();
        }
    }
}