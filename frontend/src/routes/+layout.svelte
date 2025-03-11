<script lang="ts">
	import '../app.css';
	import { base } from '$app/paths';
</script>

<div class="flex min-h-screen flex-col">
	<nav class="bg-slate-800 text-white shadow-md">
		<div class="container mx-auto px-4 py-3">
			<div class="flex items-center justify-between">
				<a href="{base}/" class="text-xl font-bold">BikeTitans</a>
				<div class="space-x-6">
					<a href="{base}/" class="transition-colors hover:text-blue-300">Home</a>
					<a href="{base}/about" class="transition-colors hover:text-blue-300">About</a>
					<a href="{base}/contact" class="transition-colors hover:text-blue-300">Contact</a>
				</div>
			</div>
		</div>
	</nav>

	<main class="container mx-auto flex-grow px-4 py-8">
		<slot />
	</main>

	<footer class="bg-slate-800 py-6 text-white">
		<div class="container mx-auto px-4 text-center">
			<p>© {new Date().getFullYear()} BikeTitans. All rights reserved.</p>
		</div>
	</footer>
</div>
