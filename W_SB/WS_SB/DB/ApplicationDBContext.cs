using Microsoft.AspNetCore.Identity.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore;

namespace WS_SB.DB
{
    public class ApplicationDbContext : DbContext
    {
        public DbSet<AccountDB> Accounts { get; set; }

        public ApplicationDbContext(DbContextOptions<ApplicationDbContext> options)
            : base(options)
        {
        }

        // 테이블 상세 설정
        protected override void OnModelCreating(ModelBuilder builder)
        {
            builder.Entity<AccountDB>()
                .HasIndex(model => model.AccountName)
                .IsUnique();
        }
    }
}
