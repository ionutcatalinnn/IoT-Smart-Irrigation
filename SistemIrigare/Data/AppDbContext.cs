using Microsoft.EntityFrameworkCore;
using SistemIrigare.Models;

namespace SistemIrigare.Data
{
    public class AppDbContext : DbContext
    {
        public AppDbContext(DbContextOptions<AppDbContext> options) : base(options) { }

        public DbSet<IrigareEvent> IrigareEvents { get; set; }
        public DbSet<SenzorDate> SenzoriDate { get; set; }
    }
}